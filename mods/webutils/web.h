#ifndef WEBUTILS_WEB_H
#define WEBUTILS_WEB_H

#include <coreutils/file.h>
#include <coreutils/log.h>
#include <string>
#include <cstdio>

#define NOGDI
#include <curl/curl.h>
#include <memory>
#include <vector>

namespace webutils {

class Web {
public:

	using StreamFunc = std::function<bool(uint8_t *, size_t)>;

	class Job {
	public:
		bool done() const { return isDone; }
		long code() const {
			long rc = -1;
			if(curl)
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
			return rc;
		}
		
		void wait(int timeout = -1);

		void stop() {
			if(curl) {
				CURLM *curlm = Web::multiHandle();
				curl_multi_remove_handle(curlm, curl);
				curl_easy_cleanup(curl);
				curl = nullptr;
				if(targetFile.exists())
					targetFile.remove();
			}
		}

		void setTarget(const utils::File &file) {
			targetFile = file;
		}

		void setUrl(const std::string &url) {
			this->url = url;
		}

		void setStreamCallback(StreamFunc cb) {
			streamCb = cb;
		}

		std::string textResult() const {
			// TODO: At _least_ UTF8 support here
			return std::string(data.begin(), data.end());
		}

	protected:

		void start() {

			CURLM *curlm = Web::multiHandle();
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
		}

		static size_t writeFunc(void *ptr, size_t size, size_t x, void *userdata) {
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

		static size_t headerFunc(char *text, size_t size, size_t n, void *userdata) {
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

		void finish() {
			isDone = true;
			auto rc = code();
			if(targetFile) {
				if(rc != 200) {
					if(targetFile.exists())
						targetFile.remove();
					return;
				} else {
					targetFile.close();
					targetFile.rename(orgFile);
				}
			}
			if(streamCb)
				streamCb(nullptr, 0);
			call_handler();
			targetFile = utils::File();
			if(curl)
				curl_easy_cleanup(curl);
			curl = nullptr;
		}

		virtual void call_handler() {}

		CURL *curl = nullptr;

		std::string url;
		std::vector<uint8_t> data;
		utils::File targetFile;
		utils::File orgFile;
		StreamFunc streamCb;
		bool isDone = false;

		friend Web;
	};
	
	template <typename ... ARGS> struct JobImpl;

	template <typename FX, typename A0> struct JobImpl<void (FX::*)(A0) const>;

	template <typename FX> struct JobImpl<FX, void (FX::*)() const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override {
			cb();
		}
		FX cb;
	};

	template <typename FX> struct JobImpl<FX, void (FX::*)(Job&) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override {
			cb(*this);
		}
		FX cb;
	};

	template <typename FX> struct JobImpl<FX, void (FX::*)(const std::string &contents) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override {
			cb(textResult());
		}
		FX cb;
	};

	template <typename FX> struct JobImpl<FX, void (FX::*)(Job&, const std::string &contents) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override {
			cb(*this, textResult());
		}
		FX cb;
	};

	template <typename FX> struct JobImpl<FX, void (FX::*)(utils::File) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override {
			cb(targetFile);
		}
		FX cb;
	};

	Web(const std::string &cacheDir = "", const std::string &baseUrl = "") : 
		cacheDir(cacheDir), baseUrl(baseUrl) {}

	static int inProgress() {
		return lastCount;	
	}


	template <typename FX> static std::shared_ptr<Job> get_url(const std::string &url, FX cb) {
		auto job = std::make_shared<JobImpl<FX, decltype(&FX::operator())>>(cb);
		job->setUrl(url);
		job->start();
		jobs.push_back(job);
		return job;
	}

	static void pollAll() {

		auto it = jobs.begin();
		while(it != jobs.end()) {
			if(it->get()->curl == nullptr) {
				it = jobs.erase(it);
			} else
				it++;
		}

		int handleCount;
		CURLM *mh = multiHandle();
		CURLMcode rc = CURLM_CALL_MULTI_PERFORM;
		while(rc == CURLM_CALL_MULTI_PERFORM)
			rc = curl_multi_perform(mh, &handleCount);
		
		lastCount = handleCount;
		CURLMsg *msg;
		std::vector<std::shared_ptr<Job>> toRemove;
		do {
			int count;
			if((msg = curl_multi_info_read(mh, &count))) {
				if(msg->msg == CURLMSG_DONE) {
					auto it = jobs.begin();
					while(it != jobs.end()) {
						if(it->get()->curl == msg->easy_handle) {
							toRemove.push_back(*it);
							it = jobs.erase(it);
						} else
							it++;
					}

				}
			}

		} while(msg);

		for(auto &r : toRemove) {
			r->finish();
		}
	}

	template <typename FX> std::shared_ptr<Job> getFile(const std::string &url, FX cb) {
		auto job = std::make_shared<JobImpl<FX, decltype(&FX::operator())>>(cb);
		auto target = cacheDir / utils::urlencode(baseUrl + url, ":/\\?;");
		job->setTarget(target);
		job->setUrl(url);
		job->start();
		jobs.push_back(job);
		return job;	
	}

	bool inCache(const std::string &url) const {
		auto target = cacheDir / utils::urlencode(baseUrl + url, ":/\\?;");
		return utils::File::exists(target);
	}

	std::shared_ptr<Job> streamData(const std::string &url, StreamFunc cb) {
		auto job = std::make_shared<Job>();
		job->setStreamCallback(cb);
		job->setUrl(url);
		job->start();
		jobs.push_back(job);
		return job;	
	}

private:

	std::string baseUrl;
	utils::File cacheDir;

	static CURLM *multiHandle() {
		static CURLM *handle = nullptr;
		if(!handle) {
			curl_global_init(CURL_GLOBAL_ALL);
			handle = curl_multi_init();
		}
		return handle;
	}

	static std::vector<std::shared_ptr<Web::Job>> jobs;
	static int lastCount;

};



} // namespace webutils

#endif // WEBUTILS_WEB_H

