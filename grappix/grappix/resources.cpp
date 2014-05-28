#include "resources.h"

using namespace image;

#ifdef EMSCRIPTEN

#include "resources.h"

#include <coreutils/utils.h>
#include <coreutils/format.h>
#include <coreutils/log.h>


using namespace std;
using namespace utils;

namespace grappix {

Resources::Resources() {
}

Resources::~Resources() {
}

//Resources::Resources(const Resources &) = delete;

void Resources::register_image(const string &name, function<void(bitmap &b)> generator) {

	string fname = format("resources/%s.png", name);
	Resource res;
	res.name = name;
	res.file_name = fname;
	res.generator = generator;

	if(File::exists(fname)) {
		res.image = load_png(fname);
	} else {
		//auto res = images.emplace(piecewise_construct, forward_as_tuple(name), forward_as_tuple(w, h));
		generator(res.image);
		//save_png(res.image, fname);
	}
	res.loaded = true;
	resources[name] = res;
};

void Resources::on_load(const string &name, function<void(const string &name, Resources&)> f) {
	auto &r = resources[name];
	r.on_load = f;
	if(r.loaded)
		f(name, *this);
}

bool Resources::done() { return true; }

const bitmap& Resources::get_image(const string &name) {
	const auto &r = resources[name];
	return r.image;
}

void Resources::update() {
}

}
//#elif defined ANDROID


#else

#include <sys/inotify.h>
#include <sys/select.h>

#include <coreutils/utils.h>
#include <coreutils/format.h>
#include <coreutils/log.h>


using namespace std;
using namespace utils;

namespace grappix {

Resources::Resources() : delay_counter(0) {
	makedir("resources");
	infd = inotify_init();
	LOGD("INOTIFY FD = %d", infd);
	//watchfd = inotify_add_watch(infd, "resources", IN_CREATE|IN_DELETE|IN_CLOSE_WRITE);
}

Resources::~Resources() {
}

//Resources::Resources(const Resources &) = delete;

void Resources::register_image(const string &name, function<void(bitmap &b)> generator) {

	string fname = format("resources/%s.png", name);
	Resource res;
	res.name = name;
	res.file_name = fname;
	res.generator = generator;

	if(File::exists(fname)) {
		res.image = load_png(fname);
	} else {
		//auto res = images.emplace(piecewise_construct, forward_as_tuple(name), forward_as_tuple(w, h));
		generator(res.image);
		save_png(res.image, fname);
	}
	res.loaded = true;
	resources[name] = res;
};

void Resources::load_text(const std::string &fileName, std::function<void(const std::string &contents)> func) {
	Resource res;
	res.name = fileName;
	res.file_name = fileName;
	res.on_load = func;
	res.loaded = true;
	res.watchfd = inotify_add_watch(infd, fileName.c_str(), IN_DELETE_SELF|IN_CLOSE_WRITE|IN_MOVE_SELF);
	LOGD("WATCH %d added", res.watchfd);

	auto dn = path_directory(fileName);
	if(dirnames.count(dn) == 0) {
		LOGD("WATCHING %s", dn);
		dirnames[dn] = inotify_add_watch(infd, dn.c_str(), IN_CREATE|IN_DELETE|IN_MOVED_TO|IN_CLOSE_WRITE);
	}

	resources[res.name] = res;
	File f { fileName };
	auto s = f.read();
	f.close();
	res.on_load(s);
}

void Resources::on_load(const string &name, function<void(const string &name)> f) {
	auto &r = resources[name];
	r.on_load = f;
	if(r.loaded)
		f(name);
}

bool Resources::done() { return true; }

const bitmap& Resources::get_image(const string &name) {
	const auto &r = resources[name];
	return r.image;
}

void Resources::update() {
	if(delay_counter++ < 30)
		return;
	delay_counter = 0;
	fd_set readset;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1;
	FD_ZERO(&readset);
	FD_SET(infd, &readset);
	//LOGD("Checking resources");
	int sr = select(infd+1, &readset, nullptr, nullptr, &tv);
	if(sr > 0) {
		static vector<uint8_t> buffer(1024);
		LOGD("Got event");
		uint8_t *ptr = &buffer[0];
		int rc = read(infd, ptr, 1024);
		LOGD("%d", rc);
		while(rc > 0) {
			inotify_event *evt = (inotify_event*)ptr;
			LOGD("Modified:%x %s", evt->mask, evt->name);
			//auto fn = format("resources/%s", evt->name);
			string fn;
			for(auto &d : dirnames) {
				if(evt->wd == d.second) {
					fn = format("%s/%s", d.first, evt->name);
					break;
				}
			}
			if(fn != "") {
				for(auto &p : resources) {
					auto &r = p.second;
					if(r.file_name == fn) {
						LOGD("Reloading resource %s", r.file_name);
						//r.image = load_png(r.file_name);
						//r.on_load(r.name, *this);
						File f { r.file_name };
						auto s = f.read();
						r.on_load(s);
					}
				}
			} else {
				for(auto &p : resources) {
					auto &r = p.second;
					if(r.watchfd == evt->wd) {
						LOGD("Reloading resource %s", r.file_name);
						//r.image = load_png(r.file_name);
						//r.on_load(r.name, *this);
						File f { r.file_name };
						auto s = f.read();
						r.on_load(s);
					}
				}
			}


			rc -= (sizeof(inotify_event) + evt->len);
			ptr += (sizeof(inotify_event) + evt->len);
		}
	}
}

}

#endif