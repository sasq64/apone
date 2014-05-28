#ifndef RESOURCES_H
#define RESOURCES_H

#include <image/image.h>
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

	void register_image(const std::string &name, std::function<void(image::bitmap &b)> generator);
	void on_load(const std::string &name, std::function<void(const std::string &name)> f);
	bool done();
	const image::bitmap& get_image(const std::string &name);
	void update();

	void load_text(const std::string &fileName, std::function<void(const std::string &contents)>);

private:

	struct Resource {
		std::string name;
		std::string file_name;
		int watchfd;
		std::function<void(image::bitmap &b)> generator;
		std::function<void(const std::string &name)> on_load;
		//std::vector<uint8_t> data;
		image::bitmap image;
		bool loaded;
	};

	std::unordered_map<std::string, Resource> resources;
	std::unordered_map<std::string, int> dirnames;
#ifndef EMSCRIPTEN
	int infd;
	int watchfd;
	int delay_counter;
#endif
};

}

#endif // RESOURCES_H
