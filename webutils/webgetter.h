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

	static void getURLData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)>);
	static int inProgress() { return ongoingCalls; }

	WebGetter(const std::string &workDir, const std::string &base = "");

	void setBaseURL(const std::string &base) { baseURL = base; }
	bool inCache(const std::string &url) const;


	void setErrorCallback(std::function<void(int code, const std::string &msg)> cb) {
		errorCallback = cb;
	}

	//void getURLData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)>);
	void getFile(const std::string &url, std::function<void(const utils::File&)> callback);
	void streamData(const std::string &url, std::function<void(uint8_t* data, int size)> callback);

private:

	class Job;

	Job* getURL(const std::string &url);

	void getURL(const std::string url, std::function<void(const Job&)>);

	static std::atomic<int> ongoingCalls;

	std::atomic<int> counter;
	std::future<void> f[4];
	static std::atomic<int> scounter;
	static std::future<void> sf[4];

	std::string workDir;
	std::string baseURL;
	std::function<void(int code, const std::string &msg)> errorCallback;
};

#endif // WEBGETTER_H

