
#include "web.h"


namespace webutils {

//std::vector<std::shared_ptr<Web::Job::Job>> Web::Job::jobs;
std::atomic<int> Web::runningJobs(0);
std::mutex Web::sm;
bool Web::initDone = false;



void Web::Job::start(CURLM *curlm) {

	curl = curl_easy_init();

	if(targetFile) {
		orgFile = targetFile;
		targetFile = targetFile + ".download";
	}

	auto u = utils::urlencode(url, " #");

	LOGD("Getting %s", url);
	curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunc);
	curl_multi_add_handle(curlm, curl);

	{
		std::lock_guard<std::mutex> lock(Web::sm);
		Web::runningJobs += 1;
	}
}

size_t Web::Job::writeFunc(void *ptr, size_t size, size_t x, void *userdata) {
	Job* job = static_cast<Job*>(userdata);
	size *= x;

	if(job->targetFile) {
		job->targetFile.write(static_cast<uint8_t*>(ptr), size);
	} else if(job->streamCb) {
		job->streamCb(static_cast<uint8_t*>(ptr), size);
	} else {
		unsigned pos = job->data.size();
		job->data.resize(pos + size);
		memcpy(&job->data[pos], ptr, size);
	}
	return size;
}

size_t Web::Job::headerFunc(char *text, size_t size, size_t n, void *userdata) {
	Job* job = static_cast<Job*>(userdata);
	size *= n;
	int sz = size-1;
	while(sz > 0 && (text[sz-1] == '\n' || text[sz-1] == '\r'))
		sz--;

	auto line = std::string(text, sz);

	LOGV("HEADER: '%s'", line);
	if(line.substr(0, 15) == "Content-Length:") {
		int sz = std::stol(line.substr(16));
		if(sz > 0 && job->streamCb)
			job->streamCb(nullptr, sz);
	} else
	if(line.substr(0, 9) == "Location:") {
		std::string newUrl = line.substr(10);
		LOGD("Redirecting to %s", newUrl);
		std::string newTarget = utils::urlencode(newUrl, ":/\\?;");
		// TODO: Some way to simulate symlinks on win?
#ifndef _WIN32
		symlink(newTarget.c_str(), job->targetFile.getName().c_str());
#endif
	}

	return size;
}

void Web::Job::finish() {
	isDone = true;
	auto rc = code();
	if(targetFile) {
		if(rc != 200) {
			if(targetFile.exists())
				targetFile.remove();
			targetFile = utils::File();
		} else {
			targetFile.close();
			if(orgFile) {
				if(orgFile.exists())
					orgFile.remove();
				targetFile.rename(orgFile);
			}
		}
	}
	if(streamCb)
		streamCb(nullptr, 0);
	call_handler();
	targetFile = utils::File();
	destroy();
}

void Web::Job::destroy() {
	if(curl) {
		curl_easy_cleanup(curl);
		std::lock_guard<std::mutex> lock(Web::sm);
		Web::runningJobs -= 1;
	}
	curl = nullptr;
}





} // namespace webutils

