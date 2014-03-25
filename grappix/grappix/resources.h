#ifndef RESOURCES_H
#define RESOURCES_H

#include "image.h"
#include <unordered_map>

namespace grappix {

class Resources {
public:

	static Resources& getInstance() {
		static Resources r;
		return r;
	}

	Resources();
	~Resources();
	Resources(const Resources &) = delete;

	void register_image(const std::string &name, std::function<void(bitmap &b)> generator);
	void on_load(const std::string &name, std::function<void(const std::string &name, Resources&)> f);
	bool done();
	const bitmap& get_image(const std::string &name);
	void update();

private:

	struct Resource {
		std::string name;
		std::string file_name;
		std::function<void(bitmap &b)> generator;
		std::function<void(const std::string &name, Resources&)> on_load;
		//std::vector<uint8_t> data;
		bitmap image;
		bool loaded;
	};

	std::unordered_map<std::string, Resource> resources;
	int infd;
	int watchfd;
	int delay_counter;
};

}

#endif // RESOURCES_H
