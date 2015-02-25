
#include "webgetter.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>

#include <curl/curl.h>
#include <cstring>
#include <unistd.h>
#include <future>
using namespace utils;
using namespace logging;
using namespace std;

std::atomic<int> WebGetter::scounter(0);
std::future<void> WebGetter::sf[4];

std::atomic<int> WebGetter::ongoingCalls;

class WebGetter::Job {
public:
	Job() : datapos(0) {}
	Job(const std::string &url, const std::string &targetDir);
	~Job();

	void setTargetDir(const std::string &wd) {
		targetDir = wd;
		datapos = -1;
	}

	bool isDone() const;
	int getReturnCode() const;
	std::string getFile() const;
	const std::vector<uint8_t>& getData() const { return data; }
	void urlStream(const std::string &url, std::function<void(uint8_t*, int)> cb);
	void urlGet(const std::string &url);
private:
#ifdef EMSCRIPTEN
	static void onLoad(void *arg, const char *name);
	static void onError(void *arg, int code);
#else
	static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata);
	static size_t headerFunc(void *ptr, size_t size, size_t nmemb, void *userdata);

	mutable std::mutex m;
	std::thread jobThread;
#endif
	bool loaded;
	int returnCode;
	std::string targetDir;
	std::unique_ptr<utils::File> file;
	std::vector<uint8_t> data;
	int32_t datapos;
	std::string target;
	std::function<void(uint8_t *ptr, int size)> streamCallback;
};


WebGetter::Job::Job(const string &url, const string &targetDir) : loaded(false), targetDir(targetDir), datapos(-1) {
	LOGD("Job created");
	jobThread = thread {&Job::urlGet, this, url};
}

WebGetter::Job::~Job() {
	if(jobThread.joinable())
		jobThread.join();
}

bool WebGetter::Job::isDone() const {
	lock_guard<mutex>{m};
	return loaded;
}

int WebGetter::Job::getReturnCode() const {
	lock_guard<mutex>{m};
	return returnCode;
}

string WebGetter::Job::getFile() const {
	return target;
}

void WebGetter::Job::urlStream(const std::string &url, std::function<void(uint8_t*, int)> cb) {

	streamCallback = cb;
	urlGet(url);
}

void WebGetter::Job::urlGet(const std::string &url) {

	target = targetDir + "/" + urlencode(url, ":/\\?;");

	//LOGD("TARGET:%s", target);


	int rc = 0;
	if(!File::exists(target)) {

		file = make_unique<File>(target + ".download", File::WRITE);

		ongoingCalls++;

		auto u = urlencode(url, " #");

		LOGD("Downloading %s", url);
		CURL *curl;
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunc);
		rc = curl_easy_perform(curl);
		LOGV("Curl returned %d", rc);
		ongoingCalls--;
		file->close();
		file->rename(target);
		curl_easy_cleanup(curl);
	} else {
		LOGI("Getting %s from cache", target);
		if(streamCallback) {
			LOGD("Streaming from file, YAY");
			vector<uint8_t> buffer(4096);
			File f { target };
			streamCallback(nullptr, f.getSize());
			while(true) {
				int rc = f.read(&buffer[0], buffer.size());
				if(rc <= 0)
					break;
				streamCallback(&buffer[0], rc);
			}
			f.close();
		}
	}
	if(streamCallback)
		streamCallback(nullptr, 0);

	m.lock();
	returnCode = rc;
	loaded = true;
	m.unlock();

}

size_t WebGetter::Job::writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata) {

	Job *job = (Job*)userdata;

	if(job->datapos >= 0) {
		auto &v = job->data;
		v.resize(v.size() + size * nmemb);
		memcpy(&v[job->datapos], ptr, size * nmemb);
		job->datapos += (size * nmemb);
	}

	if(job->streamCallback)
		job->streamCallback(static_cast<uint8_t*>(ptr), size * nmemb);
	if(job->file)
		job->file->write(static_cast<uint8_t*>(ptr), size * nmemb);
	return size * nmemb;
}

size_t WebGetter::Job::headerFunc(void *ptr, size_t size, size_t nmemb, void *userdata) {
	auto sz = size * nmemb;
	char *text = (char*)ptr;
	Job *job = static_cast<Job*>(userdata);

	while(sz > 0 && (text[sz-1] == '\n' || text[sz-1] == '\r'))
		sz--;

	auto line = string(text, sz);

	LOGV("HEADER:'%s'", line);
	if(line.substr(0, 15) == "Content-Length:") {
		int sz = stol(line.substr(16));
		if(sz > 0 && job->streamCallback)
			job->streamCallback(nullptr, sz);
	} else
	if(line.substr(0, 9) == "Location:") {
		string newUrl = line.substr(10);
		LOGD("Redirecting to %s", newUrl);
		string newTarget = urlencode(newUrl, ":/\\?;");
#ifdef _WIN32 // TODO: Some way to simulate symlinks?
		File::copy(job->target, newTarget);
#else
		symlink(newTarget.c_str(), job->target.c_str());
#endif
		job->target = job->targetDir + "/" + newTarget;
		job->file = make_unique<File>(job->target +".download", File::WRITE);
	}

	return size *nmemb;
}

WebGetter::WebGetter(const string &workDir, const string &base) : workDir(workDir), baseURL(base) {
	makedir(workDir.c_str());
	counter = 0;
}

WebGetter::Job* WebGetter::getURL(const string &url) {
	return new Job(baseURL + url, workDir);
}

void WebGetter::getURLData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)> callback) {
	sf[scounter] = std::async(std::launch::async, [&]() {
		try {
			//vector<uint8_t> data;
			Job job;
			job.urlGet(url);
			callback(job.getData());
		} catch(std::exception &e) {
			std::terminate();
		}
	});
	scounter = (scounter+1)%4;
}

bool WebGetter::inCache(const std::string &url) const {
	auto target = workDir + "/" + urlencode(baseURL + url, ":/\\?;");
	return File::exists(target);
}

void WebGetter::getURL(const std::string url, std::function<void(const Job &job)> callback) {
	f[counter] = std::async(std::launch::async, [=]() {
		try {
			Job job;
			job.setTargetDir(workDir);
			job.urlGet(baseURL + url);
			callback(job);
		} catch(std::exception &e) {
			std::terminate();
		}
	});
	counter = (counter+1)%4;
}

//void getURLData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)>) {
//}

void WebGetter::getFile(const std::string &url, std::function<void(const File&)> callback) {
	f[counter] = std::async(std::launch::async, [=]() {
		try {
			Job job;
			job.setTargetDir(workDir);
			job.urlGet(baseURL + url);
			if(job.getReturnCode() == CURLE_OK)
				callback(File { job.getFile() });
			else if(errorCallback)
				errorCallback(job.getReturnCode(), "");
		} catch(std::exception &e) {
			std::terminate();
		}
	});
	counter = (counter+1)%4;
}

void WebGetter::streamData(const std::string &url, std::function<void(uint8_t* data, int size)> callback) {
	f[counter] = std::async(std::launch::async, [=]() {
		try {
			Job job;
			job.setTargetDir(workDir);
			job.urlStream(baseURL + url, callback);
			callback(nullptr, job.getReturnCode());
			if((job.getReturnCode() != CURLE_OK) && errorCallback)
				errorCallback(job.getReturnCode(), "");

		} catch(std::exception &e) {
			std::terminate();
		}
	});
	counter = (counter+1)%4;
}

