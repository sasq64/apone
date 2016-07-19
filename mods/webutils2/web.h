#pragma once

#include <coreutils/utils.h>
#include <coreutils/log.h>
#include <cstdio>
#include <string>

#define NOGDI
#include <curl/curl.h>
#include <memory>
#include <mutex>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace webutils {

// One CURLM per thread
extern __thread CURLM *curlm;

struct BaseJob {
	virtual void onDone() = 0;
};


// A 'Sink' is the endpoint of a web transaction.
// It's what writes the incoming data to File or memory.
template <typename RESULT> struct Sink
{
	virtual size_t write(uint8_t *ptr, size_t size) { 
		unsigned pos = data.size();
		data.resize(pos + size);
		memcpy(&data[pos], ptr, size);
		return size;
	}

	RESULT result() {
		res = RESULT((char*)&data[0], data.size());
		return res;
	}

	RESULT res;
	std::vector<uint8_t> data;
};

/// Represents a single web transaction
template <typename RESULT>
struct Job : public BaseJob {

	// Convenience functions for setting curl options
	void set_opt() {}

	template <typename T, typename... A>
	void set_opt(CURLoption what, T value, const A& ...args) {
		curl_easy_setopt(curl.get(), what, value);
		set_opt(args...);
	}

	template <typename T, typename... A>
	void set_opt(CURLoption what, std::shared_ptr<T> value, const A& ...args) {
		curl_easy_setopt(curl.get(), what, value.get());
		set_opt(args...);
	}

	template <typename... A>
	void set_opt(CURLoption what, const std::string &value, const A& ...args) {
		curl_easy_setopt(curl.get(), what, value.c_str());
		set_opt(args...);
	}

	void init() {
		curl = std::shared_ptr<CURL>(curl_easy_init(), &curl_easy_cleanup);
		tid = std::this_thread::get_id();
		auto u = utils::urlencode(url, " #()");

		curl_slist *slist = NULL;

		slist = curl_slist_append(slist, "Icy-MetaData: 1");
		slist = curl_slist_append(
		    slist, "Accept: audio/mpeg, audio/x-mpeg, audio/mp3, audio/x-mp3, audio/mpeg3, "
		           "audio/x-mpeg3, audio/mpg, audio/x-mpg, audio/x-mpegaudio, "
		           "application/octet-stream, audio/mpegurl, audio/mpeg-url, audio/x-mpegurl, "
		           "audio/x-scpls, audio/scpls, application/pls, application/x-scpls, */*");
		header_list = std::shared_ptr<curl_slist>(slist, &curl_slist_free_all);

		slist = curl_slist_append(NULL, "ICY 200 OK");
		alias_list = std::shared_ptr<curl_slist>(slist, &curl_slist_free_all);

		set_opt(
			CURLOPT_URL, u,
			CURLOPT_HTTPHEADER, header_list,
			CURLOPT_HTTP_VERSION,CURL_HTTP_VERSION_1_0,
			CURLOPT_HTTP200ALIASES, alias_list,
			CURLOPT_SSL_VERIFYPEER, 0,
			CURLOPT_FOLLOWLOCATION, 1,
			CURLOPT_WRITEDATA, this,
			CURLOPT_WRITEFUNCTION, writeFunc,
			CURLOPT_HEADERDATA, this,
			CURLOPT_HEADERFUNCTION, headerFunc
		);
		LOGD("Curl Getting %s", u);
	}

	Job(const std::string &url) : url(url) { 
		init();
	}
	
	template <typename ... ARGS>
	Job(const std::string &url, ARGS& ... args) : url(url), sink(std::forward<ARGS>(args)...) { 
		init();
	}

	~Job() {
		LOGD("Job destructor");
	}

	/// Create a web Job. Do not call the constructor directly, since we need to `attach` the
	// shared pointer to the CURL handle, keeping the job alive even if it's not referenced
	// by application code.
	template <typename ... ARGS>
	static std::shared_ptr<Job> Create(const std::string url, ARGS&& ... args) {
		auto job = std::make_shared<Job>(url, std::forward<ARGS>(args)...);
		if(!curlm)
			curlm = curl_multi_init();
		LOGD("Adding to CURLM");
		curl_multi_add_handle(curlm, job->curl.get());
		curl_easy_setopt(job->curl.get(), CURLOPT_PRIVATE, new std::shared_ptr<Job>(job));
		return job;
	}

	/// Make sure the transfer is done, blocking if necessary
	void finish() {
		if(!done) {
			detachFromCurl();
			curl_easy_perform(curl.get());
			onDone();
		}
	}

	long code() const {
		long rc = -1;
		if(curl)
			// mutex
			curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &rc);
		return rc;
	}

	virtual void onDone() override {
		detachFromCurl();
		auto rc = code();
		done = true;
		if(rc != 200) {
			if(error_cb)
				error_cb(rc, "");
			else if(done_cb)
				done_cb(res);
		} else {
			res = sink.result();
			if(done_cb) {
				done_cb(res);
			}
		}
	}

	void pause(bool on) {
		// mutex
		curl_easy_pause(curl.get(), on ? CURLPAUSE_ALL : CURLPAUSE_CONT);
	}

	RESULT& result() {
		finish();
		return res;
	}

	RESULT& target() {
		return sink.result();
	}

	void onDone(const std::function<void(RESULT&)> &cb) {
		done_cb = cb;
	}

	void cancel() {
		detachFromCurl();
	}

	std::string getHeader(const std::string &name) {
		return headers[name];
	}

	size_t contentLength;

private:
	void detachFromCurl() {
		std::shared_ptr<BaseJob> *job;
		curl_easy_getinfo(curl.get(), CURLINFO_PRIVATE, (char**)&job);
		if(job) {
			curl_easy_setopt(curl.get(), CURLOPT_PRIVATE, nullptr);
			curl_multi_remove_handle(curlm, curl.get());
			delete job;
		}
	}

	void header(const std::string &line) {
		// Header mutex
		auto parts = utils::split(line, ":");
		if(parts.size() == 2) {
			parts[1] = utils::lstrip(parts[1], ' ');
			parts[1] = utils::rstrip(parts[1], '\n');
			headers[parts[0]] = parts[1];
			if(parts[0] == "Content-Length") {
				contentLength = std::stol(parts[1]);
			} else
			if(parts[0] == "Location") {
				std::string newUrl = parts[1];
				LOGD("Redirecting to %s", newUrl);
			}
		}
		//LOGD("HEADER: '%s'", line);
	}

	static size_t writeFunc(void *ptr, size_t size, size_t x, void *userdata) {
		// If pollThread, save this data
		// fifo.put() ; return
		int rc = static_cast<Job *>(userdata)->sink.write((uint8_t *)ptr, size * x);
		if(rc == 0)
			return CURL_WRITEFUNC_PAUSE;
		return rc;
	}

	static size_t headerFunc(char *text, size_t size, size_t n, void *userdata) {
		size_t sz = size * n;
		static_cast<Job *>(userdata)->header(std::string(text, sz));
		return sz;
	}

	Sink<RESULT> sink;
	RESULT res;

	// curl handle is removed from CURLM, result is available
	bool done = false;

	std::unordered_map<std::string, std::string> headers;
	std::string url;

	std::thread::id tid;

	std::shared_ptr<curl_slist> header_list;
	std::shared_ptr<curl_slist> alias_list;
	
	std::function<void(RESULT&)> done_cb;
	std::function<void(int code, std::string msg)> error_cb;

	std::shared_ptr<CURL> curl;
};

/// This is the type returned by get() and the intrface to the ongoing Job.
template <typename RESULT> struct WebResult {

	WebResult(std::shared_ptr<Job<RESULT>> job = nullptr) : job(job) {}

	RESULT& result() { return job->result(); }
	operator RESULT&() { return job->result(); }

	RESULT& target() { return job->target(); }
	void pause(bool on) { job->pause(on); }
	void cancel() { job->cancel(); }
	size_t contentLength() { return job->contentLength; }
	std::string getHeader(const std::string &name) {
		return job->getHeader(name);
	}

	WebResult& onDone(const std::function<void(RESULT&)> &cb) { 
		job->onDone(cb);
		return *this;
	}

	void finish() { job->finish(); }

	operator bool() {
		return job != nullptr;
	}

private:
	std::shared_ptr<Job<RESULT>> job;
};


template <typename RESULT, typename ... ARGS>
WebResult<RESULT> get(const std::string &url, ARGS ... args) {
	return WebResult<RESULT>(Job<RESULT>::Create(url, args...));
}

int poll();

} // namespace webutils

