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
	watchfd = inotify_add_watch(infd, "resources", IN_CREATE|IN_DELETE|IN_CLOSE_WRITE);
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
	if(delay_counter++ < 30)
		return;
	delay_counter = 0;
	fd_set readset;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1;
	FD_ZERO(&readset);
	FD_SET(infd, &readset);
	int sr = select(infd+1, &readset, nullptr, nullptr, &tv);
	if(sr > 0) {
		inotify_event *evt = (inotify_event*)malloc(1024);
		int rc = read(infd, evt, 1024);
		if(rc > 0) {
			LOGD("Modified:%x %s", evt->mask, evt->name);
			auto fn = format("resources/%s", evt->name);
			for(auto &p : resources) {
				auto &r = p.second;
				if(r.file_name == fn) {
					LOGD("Reloading resource");
					r.image = load_png(r.file_name);
					r.on_load(r.name, *this);
				}
			}
		}
		free(evt);
	}
}

}

#endif