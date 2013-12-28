 
#include "webrpc.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>

#include <curl/curl.h>

using namespace utils;
using namespace logging;
using namespace std;


WebRPC::Job::Job(const string &url) : done(false) {
	LOGD("Job created");
	jobThread = thread {&Job::urlCall, this, url};
}

WebRPC::Job::~Job() {
	jobThread.join();
}

bool WebRPC::Job::isDone() { 
	lock_guard<mutex>{m};
	return done;
}

string WebRPC::Job::getData() {
	lock_guard<mutex>{m};
	return data;
}

int WebRPC::Job::getReturnCode() { 
	lock_guard<mutex>{m};
	return returnCode;
}

void WebRPC::Job::urlCall(const string &url) {

	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
	int rc = curl_easy_perform(curl);
	{
		lock_guard<mutex>{m};
		returnCode = rc;
		done = true;
	}
}

size_t WebRPC::Job::writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata) {
	Job *job = (Job*)userdata;
	job->data = job->data + string((char*)ptr, size * nmemb);
	return size * nmemb;
}

/*
size_t WebRPC::Job::headerFunc(void *ptr, size_t size, size_t nmemb, void *userdata) {
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
} */

WebRPC::WebRPC(const string &baseUrl) : baseUrl(baseUrl) {
}

WebRPC::Job* WebRPC::call(const string &method) {
	return new Job(baseUrl + method);
}
