#ifndef WEBUTILS_WEB_H
#define WEBUTILS_WEB_H

#include <coreutils/file.h>
#include <coreutils/log.h>
#include <string>
#include <cstdio>

#define NOGDI
#include <curl/curl.h>
#include <unistd.h>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>


namespace webutils {

class Web {
public:

	class Job;

	using StreamFunc = std::function<bool(Job&, uint8_t *, size_t)>;

	class Job {
	public:
		bool done() const { return isDone; }
		long code() const {
			long rc = -1;
			if(curl)
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
			return rc;
		}
		
		int64_t contentLength() { return cLength; }
		
		std::string getHeader(const std::string &name) {
			return headers[name];
		}

		utils::File file() {
			return targetFile;
		}

		void wait(int timeout = -1);

		void stop() {
			stopped = true;
			if(targetFile.exists())
				targetFile.remove();
		}

		void setTarget(const utils::File &file) { targetFile = file; }

		void setUrl(const std::string &url) { this->url = url; }

		void setStreamCallback(StreamFunc cb) { streamCb = cb; }

		std::string textResult() const {
			// TODO: At _least_ UTF8 support here
			return std::string(data.begin(), data.end());
		}

	protected:
		void start(CURLM *curlm);
		static size_t writeFunc(void *ptr, size_t size, size_t x, void *userdata);
		static size_t headerFunc(char *text, size_t size, size_t n, void *userdata);
		void finish();
		void destroy();

		virtual void call_handler() {}

		CURL *curl = nullptr;
		std::unordered_map<std::string, std::string> headers;
		std::string url;
		std::vector<uint8_t> data;
		utils::File targetFile;
		utils::File orgFile;
		StreamFunc streamCb;
		bool isDone = false;
		bool stopped = false;
		int64_t cLength = 0;
		std::thread::id tid;

        std::shared_ptr<curl_slist> header_list;
        std::shared_ptr<curl_slist> alias_list;

		friend Web;
	};

	template <typename... ARGS> struct JobImpl;

	template <typename FX, typename A0> struct JobImpl<void (FX::*)(A0) const>;

	template <typename FX> struct JobImpl<FX, void (FX::*)() const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override { cb(); }
		FX cb;
	};

	template <typename FX> struct JobImpl<FX, void (FX::*)(Job) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override { cb(*this); }
		FX cb;
	};

	template <typename FX>
	struct JobImpl<FX, void (FX::*)(const std::string &contents) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override { cb(textResult()); }
		FX cb;
	};

	template <typename FX>
	struct JobImpl<FX, void (FX::*)(Job &, const std::string &contents) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override { cb(*this, textResult()); }
		FX cb;
	};

	template <typename FX> struct JobImpl<FX, void (FX::*)(utils::File) const> : public Job {
		JobImpl(FX fx) : cb(fx) {}
		void call_handler() override { cb(targetFile); }
		FX cb;
	};

	
	// Web
	
	// Sets 'baseUrl' as base for all web accesses.
	// makes sure cacheDir is created
	// Starts the worker thread
	Web(const std::string &cacheDir = "", const std::string &baseUrl = "")
	    : cacheDir(cacheDir), baseUrl(baseUrl) {
		std::lock_guard<std::mutex> lock(sm);
		if(!initDone) {
			curl_global_init(CURL_GLOBAL_ALL);
			initDone = true;
		}
        utils::makedirs(cacheDir);
		curlm = curl_multi_init();
		webThread = std::thread{&Web::run, this};
	}

	~Web() {
		quit = true;
		webThread.join();
	}

	// This is run by the worker thread and polls curl for all outstanding actions
	void run() {
		while(!quit) {
			int handleCount;
			CURLMcode rc = CURLM_CALL_MULTI_PERFORM;
			{
				std::lock_guard<std::mutex> lock(m);
				while(rc == CURLM_CALL_MULTI_PERFORM)
					rc = curl_multi_perform(curlm, &handleCount);
				lastCount = handleCount;
			}
			// TODO: Sleep longer, or use semaphore when there are no ongoing transfers
			utils::sleepms(5);
		}
	}

	static int inProgress() {
		std::lock_guard<std::mutex> lock(sm);
		return runningJobs;
	}

	template <typename FX> std::shared_ptr<Job> get(const std::string &url, FX cb) {
		auto job = std::make_shared<JobImpl<FX, decltype(&FX::operator())>>(cb);
		job->setUrl(url);
		job->start(curlm);
		jobs.push_back(job);
		return job;
	}

	void poll() {
		std::lock_guard<std::mutex> lock(m);

		auto it = jobs.begin();
		while(it != jobs.end()) {
			auto *curl = it->get()->curl;
			if(it->get()->tid != std::this_thread::get_id())
				LOGW("POLLING FROM WRONG THREAD!");
			if(curl && it->get()->stopped) {
				curl_multi_remove_handle(curlm, curl);
				it->get()->destroy();
			}
			if(!curl) {
				it = jobs.erase(it);
			} else
				it++;
		}

		CURLMsg *msg;
		std::vector<std::shared_ptr<Job>> toRemove;
		do {
			int count;
			if((msg = curl_multi_info_read(curlm, &count))) {
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
		LOGD("target: %s", target.getName());
		if(target.exists()) {
			job->call_handler();
			job->targetFile = utils::File();
			job->isDone = true;
			return job;
		}
		job->start(curlm);
		jobs.push_back(job);
		return job;
	}

	utils::File getFileBlocking(const std::string &url) {
		std::atomic<bool> done(false);
		utils::File retFile;
		auto job = getFile(url, [&](utils::File f) {
			retFile = f;
			done = true;
		});
		while(!done) {
			poll();
			utils::sleepms(100);
		}
		return retFile;
	}

	bool inCache(const std::string &url) const {
		auto target = cacheDir / utils::urlencode(baseUrl + url, ":/\\?;");
		return utils::File::exists(target);
	}

	std::shared_ptr<Job> streamData(const std::string &url, StreamFunc cb) {
		auto job = std::make_shared<Job>();
		job->setStreamCallback(cb);
		job->setUrl(url);
		job->start(curlm);
		jobs.push_back(job);
		return job;
	}

	std::shared_ptr<Job> createJob(const std::string &url) {
		auto job = std::make_shared<Job>();
		job->setUrl(url);
		job->start(curlm);
		jobs.push_back(job);
		return job;
	}

	static Web &getInstance() {
		static Web w;
		return w;
	}

	template <typename FX> static std::shared_ptr<Job> get_url(const std::string &url, FX cb) {
		std::lock_guard<std::mutex> lock(sm);
		return getInstance().get(url, cb);
	}

	static void pollAll() {
		Web &w = getInstance();
		std::lock_guard<std::mutex> lock(sm);
		w.poll();
	}

private:
	static std::mutex sm;
	std::mutex m;
	std::thread webThread;
	bool quit = false;
	std::string baseUrl;
	utils::File cacheDir;

	static bool initDone;

	CURLM *curlm = nullptr;

	std::vector<std::shared_ptr<Web::Job>> jobs;
	int lastCount;
	static std::atomic<int> runningJobs;
};

} // namespace webutils

#endif // WEBUTILS_WEB_H
