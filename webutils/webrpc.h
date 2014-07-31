#ifndef WEBRPC_H
#define WEBRPC_H

#include <coreutils/file.h>

#include <string>
#include <mutex>
#include <thread>
#include <memory>
#include <future>
#include <unordered_map>
#include <cstdio>

class WebRPC {
public:

	static std::atomic<int> ongoingCalls;

	class Job {
	public:
		Job() {};
		Job(const std::string &url, const std::string &data = "");
		~Job();
		bool isDone();
		int getReturnCode();
		std::string getData();
	//private:
		void urlCall(const std::string &url, const std::string &data = "");
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

	void setErrorCallback(std::function<void(int code, const std::string &msg)> cb) {
		errorCallback = cb;
	}

	void call(const std::string &method, const std::unordered_map<std::string, std::string>, std::function<void(const std::string &result)>);

	void call(const std::string &method, std::function<void(const std::string &result)>);
	void post(const std::string &method, const std::string &data);
	void post(const std::string &method, const std::string &data, std::function<void(const std::string &result)>);

	static int inProgress() { return ongoingCalls; }

private:
	std::string baseUrl;
	std::function<void(int code, const std::string &msg)> errorCallback;
	std::atomic<int> counter;
	std::future<void> f[4];
};

#endif // WEBRPC_H
