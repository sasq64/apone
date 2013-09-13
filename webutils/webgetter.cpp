 
#include "webgetter.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>

#include <curl/curl.h>

using namespace utils;
using namespace logging;
using namespace std;

WebGetter::Job::Job(const string &url, const string &targetDir) : loaded(false), targetDir(targetDir) {
	LOGD("Job created");
	jobThread = thread {&Job::urlGet, this, url};
}

WebGetter::Job::~Job() {
	jobThread.join();
}

bool WebGetter::Job::isDone() { 
	lock_guard<mutex>{m};
	return loaded;
}

int WebGetter::Job::getReturnCode() { 
	lock_guard<mutex>{m};
	return returnCode;
}

string WebGetter::Job::getFile() {
	return target;
}

void WebGetter::Job::urlGet(string url) {

	target = targetDir + "/" + urlencode(url, ":/\\?;");

	int rc = 0;
	if(!File::exists(target)) {

		LOGD("Downloading %s", url);
		CURL *curl;
		curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
		curl_easy_setopt(curl, CURLOPT_WRITEHEADER, this);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunc);
		rc = curl_easy_perform(curl);
		LOGD("Curl returned %d", rc);
	} else {
		LOGD("Getting %s from cache", target);
	}

	//if(fp)
	//	fclose(fp);

	m.lock();
	returnCode = rc;
	loaded = true;
	m.unlock();
}

size_t WebGetter::Job::writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata) {
	Job *job = (Job*)userdata;
	if(!job->file) {
		//job->fp = fopen(job->target.c_str(), "wb");
		job->file = make_unique<File>(job->target, File::WRITE);
		LOGD("Opened %s", job->target);
	}
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

	log(VERBOSE, "HEADER:'%s'", line);

	if(line.substr(0, 9) == "Location:") {
		string newUrl = line.substr(10);
		LOGD("Redirecting to %s", newUrl);
		job->target = job->targetDir + "/" + urlencode(newUrl, ":/\\?;");
	}

	return size *nmemb;
}

WebGetter::WebGetter(const string &workDir) : workDir(workDir) {
	makedir(workDir.c_str());
}


WebGetter::Job* WebGetter::getURL(const string &url) {
	return new Job(url, workDir);
}
