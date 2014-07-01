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

template <> std::string load_data(utils::File &f) {
	return f.read();
};


Resources::Resources() {
}

Resources::~Resources() {
}


bool Resources::done() { return true; }

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

template <> std::string load_data(utils::File &f) {
	return f.read();
};

template <> image::bitmap load_data(utils::File &f) {
	return image::load_png(f.getName());
}

template <> void save_data(utils::File &f, const std::string &data) {
	f.write(data);
}

template <> void save_data(utils::File &f, const image::bitmap &data) {
	image::save_png(data, f.getName());
}


Resources::Resources() : delay_counter(0) {
	//makedir("resources");
	infd = inotify_init();
	LOGD("INOTIFY FD = %d", infd);
	//watchfd = inotify_add_watch(infd, "resources", IN_CREATE|IN_DELETE|IN_CLOSE_WRITE);
}

Resources::~Resources() {
}

void Resources::setNotify(const std::string &fileName) {
 	auto dn = path_directory(fileName);
 	if(dirnames.count(dn) == 0) {
 		LOGD("WATCHING %s", dn);
 		dirnames[dn] = inotify_add_watch(infd, dn.c_str(), /* IN_CREATE|IN_DELETE| */ IN_MOVED_TO|IN_CLOSE_WRITE);
 	}
}

bool Resources::done() { return true; }

class Selector {
	void addFD(int fd);
	
};

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
				LOGD("Checking for %s", fn);
				if(resources.count(fn) > 0) {
					auto r = resources[fn];
					LOGD("RELOADING");
					r->load();
				}
			} 
			rc -= (sizeof(inotify_event) + evt->len);
			ptr += (sizeof(inotify_event) + evt->len);
		}
	}
}

}

#endif