#include "../window.h"
#include "../resources.h"

#include <tween/tween.h>
#include <coreutils/format.h>
#include <coreutils/utils.h>
#include <coreutils/file.h>

#include <linux/input.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unordered_map>
#include <functional>

#include <bcm_host.h>

using namespace std;
using namespace utils;

bool initEGL(EGLConfig& eglConfig, EGLContext& eglContext, EGLDisplay& eglDisplay, EGLSurface &eglSurface, EGLNativeWindowType nativeWin);

namespace grappix {

void Window::setTitle(const std::string &t) {}

Window::Window() : RenderTarget(), winOpen(false), bmCounter(0), lockIt(false) {
	NO_CLICK.x = NO_CLICK.y = NO_CLICK.button = -1;
}

void Window::open(bool fs) {
	open(0,0,fs);
}

Window::~Window() {
}

void Window::close() {
	winOpen = false;
}

static uint8_t pressed_keys[512];

Window::click Window::NO_CLICK = { -1, -1, -1};

static function<void(uint32_t)> renderLoopFunction;

static EGL_DISPMANX_WINDOW_T nativewindow;

static EGLConfig eglConfig;
static EGLContext eglContext;
static EGLDisplay eglDisplay;
static EGLSurface eglSurface;

/*constexpr */ bool test_bit(const vector<uint8_t> &v, int n) {
	return (v[n/8] & (1<<(n%8))) != 0;
}


void Window::open(int w, int h, bool fs) {

	if(winOpen)
		return;

	bcm_host_init();

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;

	uint32_t display_width;
	uint32_t display_height;

	// create an EGL window surface, passing context width/height
	int success = graphics_get_display_size(0 /* LCD */, &display_width, &display_height);
	if(success < 0)
		throw display_exception("Cound not get display size");

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = display_width;
	dst_rect.height = display_height;

	uint16_t dwa = 0;
	uint16_t dha = 0;

	// Scale 50% on hires screens
	if(display_width > 1024) {
		display_width /= 2;
		display_height /= 2;
		dwa = display_width;
		dha = display_height;
	}

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = display_width << 16 | dwa;
	src_rect.height = display_height << 16 | dha;

	dispman_display = vc_dispmanx_display_open(0 /* LCD */);
	dispman_update = vc_dispmanx_update_start(0);

	dispman_element = vc_dispmanx_element_add(dispman_update,
	dispman_display, 0/*layer*/, &dst_rect, 0/*src*/,
	&src_rect, DISPMANX_PROTECTION_NONE, nullptr /*alpha*/,
	nullptr/*clamp*/, DISPMANX_NO_ROTATE);

	nativewindow.element = dispman_element;
	nativewindow.width = display_width;
	nativewindow.height = display_height;
	vc_dispmanx_update_submit_sync(dispman_update);

	initEGL(eglConfig, eglContext, eglDisplay, eglSurface, &nativewindow);

	setup(display_width, display_height);
	memset(pressed_keys, 0, sizeof(pressed_keys));

	keyboardCount = 0;

	keyboardThread = thread([=]() {

		File idir { "/dev/input" };
		vector<uint8_t> evbit((EV_MAX+7)/8);
	    vector<uint8_t> keybit((KEY_MAX+7)/8);

		int fd = -1;
		vector<int> fdv;
		for(auto f : idir.listFiles()) {
			if(!f.isDir()) {
				fd = ::open(f.getName().c_str(), O_RDONLY, 0);
				if(fd >= 0) {
					ioctl(fd, EVIOCGBIT(0, evbit.size()), &evbit[0]);
					if(test_bit(evbit, EV_KEY)) {
						ioctl(fd, EVIOCGBIT(EV_KEY, keybit.size()), &keybit[0]);
						if(test_bit(keybit, KEY_F8))
							keyboardCount++;
						if(test_bit(keybit, KEY_LEFT) || test_bit(keybit, BTN_LEFT)) {
							ioctl(fd, EVIOCGRAB, 1);
							fdv.push_back(fd);
							continue;
						}
					}
					::close(fd);
				} else
					LOGW("Could not access %s", f.getName());
			}
			//LOGD("%s, %02x -- %02x", f.getName(), evbit, keybit);
		}

		LOGD("Found %d devices with keys", fdv.size());

		if(fdv.size() == 0)
			return;

		int maxfd = -1;

		fd_set readset;
		struct timeval tv;

		vector<uint8_t> buf(256);

		while(true) {
			FD_ZERO(&readset);
			for(auto fd : fdv) {
				FD_SET(fd, &readset);
				if(fd > maxfd)
					maxfd = fd;
			}
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			int sr = select(maxfd+1, &readset, nullptr, nullptr, &tv);
			if(sr > 0) {
				//LOGD("Got signal");
				//static uint8_t buf[2048];
				for(auto fd : fdv) {
					if(FD_ISSET(fd, &readset)) {
						int rc = read(fd, &buf[0], sizeof(struct input_event) * 4);
						auto *ptr = (struct input_event*)&buf[0];
						//if(rc >= sizeof(struct input_event))
						//	LOGD("[%02x]", buf);
						while(rc >= sizeof(struct input_event)) {
							if(ptr->type == EV_KEY) {
								//LOGD("TYPE %d CODE %d VALUE %d", ptr->type, ptr->code, ptr->value);
								if(ptr->value) {
									auto k = ptr->code;
									if(k >= KEY_1 && k <= KEY_9)
										k += ('1' - KEY_1);
									else if(k >= KEY_F1 && k <= KEY_F10)
										k += (F1-KEY_F1);
									else {
										for(auto t : Window::translate) {
											if(t.second == k) {
												k = t.first;
												break;
											}
										}
									}
									putEvent<KeyEvent>(k);
								}
								if(ptr->code < 512)
									pressed_keys[ptr->code] = ptr->value;
							}
							ptr++;
							rc -= sizeof(struct input_event);
						}
					}
				}
			}
		}
	});
	keyboardThread.detach();

	atexit([](){
		if(!renderLoopFunction) {
			while(screen.is_open()) {
				screen.update_callbacks();
				screen.flip();
			}
		}
	});
};


void Window::render_loop(function<void(uint32_t)> f, int fps) {
	renderLoopFunction = f;

	//atexit([](){
		auto lastMs = getms();
		while(screen.is_open()) {
			screen.update_callbacks();
			auto ms = getms();
			uint32_t rate = ms - lastMs;
			lastMs = ms;
			renderLoopFunction(rate);
			//while(screen.locked()) {
			//	sleepms(5);
			//}
		}
	//});
}

void Window::flip() {
	if(eglDisplay != EGL_NO_DISPLAY) {
		eglSwapBuffers(eglDisplay, eglSurface);
		//eglQuerySurface(eglDisplay, eglSurface, EGL_WIDTH, &screenWidth);
		//eglQuerySurface(eglDisplay, eglSurface, EGL_HEIGHT, &screenHeight);
	}
	//auto t = chrono::high_resolution_clock::now();
	//auto ms = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	//tween::Tween::updateTweens(ms / 1000000.0f);
	//Resources::getInstance().update();
}

bool Window::mouse_pressed() {
	return (pressed_keys[BTN_MOUSE] != 0);;
}

tuple<int, int> Window::mouse_position() {
	return make_tuple(-1,-1);
}

#define LCTRL 1
#define LSHIFT 2
#define LALT 4
#define LWIN 8
#define RCTRL 0x10
#define RSHIFT 0x20
#define RALT 0x40
#define RWIN 0x80

bool Window::key_pressed(key k) {
	auto rawkey = translate[k];
	return (pressed_keys[rawkey] != 0);
}

bool Window::key_pressed(char k) {
	int rawkey = k - 'A' + 0x4;
	return (pressed_keys[rawkey] != 0);
}

Window::click Window::get_click(bool peek) {
/*	if(click_buffer.size() > 0) {
		auto k = click_buffer.front();
		if(!peek)
			click_buffer.pop_front();
		return k;
	}*/
	return NO_CLICK;
}

unordered_map<int, int> Window::translate = {

	{ 'A', KEY_A },
	{ 'B', KEY_B },
	{ 'C', KEY_C },
	{ 'D', KEY_D },
	{ 'E', KEY_E },
	{ 'F', KEY_F },
	{ 'G', KEY_G },
	{ 'H', KEY_H },
	{ 'I', KEY_I },
	{ 'J', KEY_J },
	{ 'K', KEY_K },
	{ 'L', KEY_L },
	{ 'M', KEY_M },
	{ 'N', KEY_N },
	{ 'O', KEY_O },
	{ 'P', KEY_P },
	{ 'Q', KEY_Q },
	{ 'R', KEY_R },
	{ 'S', KEY_S },
	{ 'T', KEY_T },
	{ 'U', KEY_U },
	{ 'V', KEY_V },
	{ 'W', KEY_W },
	{ 'X', KEY_X },
	{ 'Y', KEY_Y },
	{ 'Z', KEY_Z },
	{ '0', KEY_0 },

	{ '-', KEY_MINUS },
	{ '=', KEY_EQUAL },
	{ '[', KEY_LEFTBRACE },
	{ ']', KEY_LEFTBRACE },
	{ F11, F11 },
	{ F12, F12 },
	{ BTN_LEFT, CLICK },
	{ BTN_RIGHT, RIGHT_CLICK },
	{ ENTER, KEY_ENTER },
	{ SPACE, KEY_SPACE },
	{ PAGEUP, KEY_PAGEUP },
	{ PAGEDOWN, KEY_PAGEDOWN },
	{ RIGHT, KEY_RIGHT },
	{ LEFT, KEY_LEFT },
	{ DOWN, KEY_DOWN },
	{ UP, KEY_UP },
	{ ESCAPE, KEY_ESC },
	{ BACKSPACE, KEY_BACKSPACE },
	{ DELETE, KEY_DELETE },
	{ TAB, KEY_TAB },
	{ END, KEY_END },
	{ HOME, KEY_HOME },
	{ SHIFT_LEFT, KEY_LEFTSHIFT },
	{ SHIFT_RIGHT, KEY_RIGHTSHIFT },
	{ WINDOW_LEFT, KEY_LEFTMETA },
	{ WINDOW_RIGHT, KEY_RIGHTMETA },
	{ ALT_LEFT, KEY_LEFTALT },
	{ ALT_RIGHT, KEY_RIGHTALT },
	{ CTRL_LEFT, KEY_LEFTCTRL},
	{ CTRL_RIGHT, KEY_RIGHTCTRL }
};


Window::key Window::get_key(bool peek) {
	if(hasEvents<KeyEvent>()) {
		auto ke = getEvent<KeyEvent>();
		auto k = ke.code;
		return (key)k;
	}
	return NO_KEY;
};

std::shared_ptr<Window> screenptr = make_shared<Window>();
Window& screen = *screenptr;

}
