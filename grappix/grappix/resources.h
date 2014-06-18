#ifndef RESOURCES_H
#define RESOURCES_H

#include <image/image.h>
#include <unordered_map>
#include <coreutils/file.h>

namespace grappix {

template <typename T> T load_data(utils::File &f) {
	return f.read<T>();
};

template <> std::string load_data(utils::File &f);

template <> image::bitmap load_data(utils::File &f);

template <typename T> void save_data(utils::File &f, const T &data) {
	f.write<T>();
};

template <> void save_data(utils::File &f, const std::string &dataa);

template <> void save_data(utils::File &f, const image::bitmap &data);


class Resources {
public:

	static Resources& getInstance() {
		static Resources r;
		return r;
	}

	Resources();
	~Resources();
	Resources(const Resources &) = delete;

	bool done();
	void update();

	template <typename T> void load(const std::string &fileName, std::function<void(T &data)> onLoad) {		
		auto fn = utils::File::resolvePath(fileName);
		auto r = std::make_shared<TypedResource<T>>(fn, onLoad);
		resources[fn] = r;
		r->load();
		setNotify(fn);
	}
	//template <typename T> void load(const std::string &fileName, std::function<void(T &data)> onLoad, const T& def);
	template <typename T> void load(const std::string &fileName, std::function<void(T &data)> onLoad, std::function<T()> onGenerate) {
		auto fn = utils::File::resolvePath(fileName);
		auto r = std::make_shared<TypedResource<T>>(fn, onLoad, onGenerate);
		resources[fn] = r;
		r->load();
		setNotify(fn);
	}

	template <typename T> void load(const std::string &fileName, std::function<void(T &data)> onLoad, const T& defaultVal) {
		auto fn = utils::File::resolvePath(fileName);
		auto r = std::make_shared<TypedResource<T>>(fn, onLoad, [=]() -> T { return defaultVal; });
		resources[fn] = r;
		r->load();
		setNotify(fn);
	}

	void setNotify(const std::string &fileName);

private:

	class Resource {
	public:
		virtual void load() {}
		virtual void generate() {}
	};

	template <typename T> class TypedResource : public Resource {
	public:
		TypedResource(const std::string &fileName, std::function<void(T &data)> onLoad) : file_name(fileName), on_load(onLoad) {
		}

		TypedResource(const std::string &fileName, std::function<void(T &data)> onLoad, std::function<T()> onGenerate) : 
			file_name(fileName), on_load(onLoad), on_generate(onGenerate) {
		}

		virtual void load() {
			utils::File f { file_name };
			if(f.exists()) {
				auto data = load_data<T>(f);
				on_load(data);
			} else if(on_generate) {
				auto data = on_generate();
				utils::makedirs(f.getDirectory());
				save_data(f, data);
				on_load(data);
			} else
				throw utils::file_not_found_exception(file_name);
		}

	private:
		std::string file_name;
		std::function<void(T &data)> on_load;
		std::function<T()> on_generate;
	};

	std::unordered_map<std::string, shared_ptr<Resource>> resources;
	std::unordered_map<std::string, int> dirnames;
#ifndef EMSCRIPTEN
	int infd;
	int watchfd;
	int delay_counter;
#endif
};

}

#endif // RESOURCES_H
