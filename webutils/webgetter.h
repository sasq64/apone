#ifndef WEBGETTER_H
#define WEBGETTER_H

#include <coreutils/file.h>

#include <string>
#ifdef EMSCRIPTEN
#else
#include <mutex>
#include <future>
#include <thread>
#endif
#include <memory>
#include <cstdio>
#include <functional>
#include <vector>

class WebGetter {
public:
	class Job {
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
	};

	static void getURLData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)>);

	WebGetter(const std::string &workDir) ;
	Job* getURL(const std::string &url);

	void getURL(const std::string url, std::function<void(const Job&)>);

	void setBaseURL(const std::string &base) { baseURL = base; }

	bool inCache(const std::string &url) const;
private:
	std::atomic<int> counter;
	std::future<void> f[4];
	std::string workDir;
	std::string baseURL;
};

#endif // WEBGETTER_H

