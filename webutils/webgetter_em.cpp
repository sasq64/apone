 
#include "webgetter.h"

#include <emscripten.h>

#include <coreutils/log.h>
#include <coreutils/utils.h>


using namespace utils;
using namespace logging;
using namespace std;

WebGetter::Job::Job(const string &url, const string &targetDir) : loaded(false), targetDir(targetDir) {
	LOGD("Job created");

	target = targetDir + "/" + urlencode(url, ":/\\?;");
	int rc = 0;
	if(!File::exists(target)) {
		emscripten_async_wget2(url.c_str(), target.c_str(), "GET", nullptr, (void*)this, onLoad, onError, nullptr);
	} else {
		LOGD("Getting %s from cache", target);
		returnCode = rc;
		loaded = true;
	}
}

void WebGetter::Job::onLoad(void *arg, const char *name) {
	LOGD("### Got %s", name);
	WebGetter::Job *job = (WebGetter::Job*)arg;
	job->target = name;
	job->returnCode = 0;
	job->loaded = true;
}

void WebGetter::Job::onError(void *arg, int code) {
	LOGD("Failed");
	WebGetter::Job *job = (WebGetter::Job*)arg;
	job->returnCode = -1;
	job->loaded = true;
}


WebGetter::Job::~Job() {
}

bool WebGetter::Job::isDone() { 
	return loaded;
}

int WebGetter::Job::getReturnCode() { 
	return returnCode;
}

string WebGetter::Job::getFile() {
	return target;
}

WebGetter::WebGetter(const string &workDir) : workDir(workDir) {
	makedir(workDir.c_str());
}


WebGetter::Job* WebGetter::getURL(const string &url) {
	return new Job(url, workDir);
}
