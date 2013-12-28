#ifndef WEBRPC_H
#define WEBRPC_H

#include <coreutils/file.h>

#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <stdio.h>

class WebRPC {
public:
	class Job {
	public:
		Job(const std::string &url);
		~Job();
		bool isDone();
		int getReturnCode();
		std::string getData();
	private:
		void urlCall(const std::string &url);
		static size_t writeFunc(void *ptr, size_t size, size_t nmemb, void *userdata);
		//static size_t headerFunc(void *ptr, size_t size, size_t nmemb, void *userdata);

		std::mutex m;
		bool done;
		int returnCode;
		std::thread jobThread;
		std::string data;
	};

	WebRPC(const std::string &baseURL) ;
	Job* call(const std::string &method);
private:
	std::string baseUrl;
};

#endif // WEBRPC_H
