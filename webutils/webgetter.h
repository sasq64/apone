#ifndef WEBGETTER_H
#define WEBGETTER_H

#include <coreutils/file.h>

#include <string>
#ifdef EMSCRIPTEN
#else
#include <mutex>
#include <thread>
#endif
#include <memory>
#include <stdio.h>

class WebGetter {
public:
	class Job {
	public:
		Job(const std::string &url, const std::string &targetDir);
		~Job();
		bool isDone();
		int getReturnCode();
		std::string getFile();
	private:
		void urlGet(std::string url);
#ifdef EMSCRIPTEN
		static void onLoad(void *arg, const char *name);
		static void onError(void *arg, int code);
#else
		static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata);
		static size_t headerFunc(void *ptr, size_t size, size_t nmemb, void *userdata);

		std::mutex m;
		std::thread jobThread;
#endif
		bool loaded;
		int returnCode;
		std::string targetDir;
		std::unique_ptr<utils::File> file;
		std::string target;
	};

	WebGetter(const std::string &workDir) ;
	Job* getURL(const std::string &url);
	void setBaseURL(const std::string &base) { baseURL = base; }
private:
	std::string workDir;
	std::string baseURL;
};

#endif // WEBGETTER_H

