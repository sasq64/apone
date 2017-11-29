
#include "web.h"


namespace webutils {

std::atomic<int> Web::runningWebJobs(0);
std::mutex Web::sm;
bool Web::initDone = false;

void WebJob::start(CURLM *curlm) {

	curl = curl_easy_init();

	tid = std::this_thread::get_id();

	if(targetFile) {
		orgFile = targetFile;
		targetFile = targetFile + ".download";
	}

	auto u = utils::urlencode(url, " #()");

    curl_slist *slist = NULL;

	//slist = curl_slist_append(slist, "User-Agent: chipmachine");
    slist = curl_slist_append(slist, "Icy-MetaData: 1");
	slist = curl_slist_append(slist, "Accept: audio/mpeg, audio/x-mpeg, audio/mp3, audio/x-mp3, audio/mpeg3, audio/x-mpeg3, audio/mpg, audio/x-mpg, audio/x-mpegaudio, application/octet-stream, audio/mpegurl, audio/mpeg-url, audio/x-mpegurl, audio/x-scpls, audio/scpls, application/pls, application/x-scpls, */*");  
    header_list = std::shared_ptr<curl_slist>(slist, &curl_slist_free_all);

    slist = NULL;
    slist = curl_slist_append(slist, "ICY 200 OK");
    alias_list = std::shared_ptr<curl_slist>(slist, &curl_slist_free_all);

	LOGD("Curl Getting %s", u);
	curl_easy_setopt(curl, CURLOPT_URL, u.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list.get());
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    curl_easy_setopt(curl, CURLOPT_HTTP200ALIASES, alias_list.get());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunc);
	curl_multi_add_handle(curlm, curl);

	{
		std::lock_guard<std::mutex> lock(Web::sm);
		Web::runningWebJobs += 1;
	}
}

size_t WebJob::writeFunc(void *ptr, size_t size, size_t x, void *userdata) {
	WebJob* job = static_cast<WebJob*>(userdata);
	size *= x;
	
	if(job->stopped) {
		LOGD("Job s stopped");
		return -1;
	}

	if(job->targetFile) {
		job->targetFile.write(static_cast<uint8_t*>(ptr), size);
	} else if(job->streamCb) {
		job->streamCb(*job, static_cast<uint8_t*>(ptr), size);
	} else {
		unsigned pos = job->data.size();
		job->data.resize(pos + size);
		memcpy(&job->data[pos], ptr, size);
	}
	return size;
}

size_t WebJob::headerFunc(char *text, size_t size, size_t n, void *userdata) {
	WebJob* job = static_cast<WebJob*>(userdata);
	size *= n;
	int sz = size-1;
	while(sz > 0 && (text[sz-1] == '\n' || text[sz-1] == '\r'))
		sz--;
	text[sz] = 0;
	char *split = strstr(text, ":");
	std::string name, val;
	if(!split)	
		name = std::string(text, sz);
	else {
		int pos = split-text;
		name = std::string(text, 0, pos);
		pos++;
		if(text[pos] == ' ') pos++;
		val = std::string(text, pos, sz-pos);
		job->headers[name] = val;
	}	

	LOGD("HEADER: '%s = %s'", name, val);
	if(name == "Content-Length") {
		job->cLength = std::stol(val);
	} else
	if(name== "Location") {
		std::string newUrl = val;
		LOGD("Redirecting to %s", newUrl);
		std::string newTarget = utils::urlencode(newUrl, ":/\\?;");
		// TODO: Some way to simulate symlinks on win?
#ifndef _WIN32
		symlink(newTarget.c_str(), job->targetFile.getName().c_str());
#endif
	}

	return size;
}

void WebJob::finish() {
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
		streamCb(*this, nullptr, 0);
	call_handler();
	targetFile = utils::File();
	destroy();
}

void WebJob::destroy() {
	if(curl) {
		curl_easy_cleanup(curl);
		std::lock_guard<std::mutex> lock(Web::sm);
		Web::runningWebJobs -= 1;
	}
	curl = nullptr;
}





} // namespace webutils

