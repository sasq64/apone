#ifndef NET_H
#define NET_H

#include <string>
#include <functional>
#include <vector>

namespace utils {
	class File;
}

namespace net {

class WebGetter {
public:
	WebGetter(const std::string &cacheDir) : cacheDir(cacheDir) {}

	void getFile(const std::string &url, std::function<void(const utils::File&)> callback);
	//static void getFile(const std::string &url, std::function<void(const utils::File&)> callback);
	static void getData(const std::string &url, std::function<void(const std::vector<uint8_t> &data)>);
private:
	const std::string cacheDir;
};

}

#endif // NET_H
