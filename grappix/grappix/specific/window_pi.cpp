#include "../window.h"
#include "../tween.h"
#include "../resources.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <stdio.h>
#include <unordered_map>
#include <functional>

#include <bcm_host.h>

using namespace std;

namespace grappix {

Window::Window() : RenderTarget(), winOpen(false), bmCounter(0), lockIt(false) {
	NO_CLICK.x = NO_CLICK.y = NO_CLICK.button = -1;
}

void Window::open(bool fs) {
	open(0,0,fs);
}

Window::click Window::NO_CLICK = { -1, -1, -1};

static function<void(uint32_t)> renderLoopFunction;

static EGL_DISPMANX_WINDOW_T nativewindow;


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

	// You can hardcode the resolution here:
	//display_width = 640;
	//display_height = 480;

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = display_width;
	dst_rect.height = display_height;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = display_width << 16;
	src_rect.height = display_height << 16;

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

	update_matrix();
	glLineWidth(2.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	lastTime = -1;
	winOpen = true;

	//glfwSetKeyCallback(key_fn);


	startTime = chrono::high_resolution_clock::now();
	frameBuffer = 0;

	atexit([](){
		if(!renderLoopFunction) {
			while(screen.is_open()) {
				screen.update_callbacks();
				screen.flip();
			}
		}	
	});
};


int Window::call_repeatedly(std::function<void(void)> f, int msec) {
	callbacks.push_back(Callback(f, msec));
	return callbacks.size()-1;
}

void Window::update_callbacks() {

	while(safeFuncs.size() > 0) {
		safeMutex.lock();
		auto &f = safeFuncs.front();
		f();
		safeFuncs.pop_front();
		safeMutex.unlock();
	}

	auto ms = utils::getms();
	for(auto &cb : callbacks) {
		if(cb.msec == 0 || ms >= cb.next_time) {
			cb.cb();
			cb.next_time += cb.msec;
		}
	}

	for(auto i : to_remove) {
		callbacks.erase(callbacks.begin() + i);
	}
	to_remove.clear();
}

void Window::remove_repeating(int index) {
	to_remove.insert(index);
}

void Window::render_loop(function<void(uint32_t)> f, int fps) {
	renderLoopFunction = f;

	atexit([](){
		auto lastMs = utils::getms();
		while(screen.is_open()) {
			screen.update_callbacks();
			auto ms = utils::getms();
			uint32_t rate = ms - lastMs;
			lastMs = ms;
			renderLoopFunction(rate);
			//while(screen.locked()) {
			//	utils::sleepms(5);
			//}
		}
	});
}

void Window::vsync() {
}

//static uint64_t lastTime;

void Window::flip() {
	auto t = chrono::high_resolution_clock::now();
#ifdef FPS_COUNTER
	auto tm = utils::getms();
	auto d = tm - lastTime;
	if(d > 0)
		fps = fps * 0.8 + (1000 / d) * 0.2;
	lastTime = tm;
	text(utils::format("%d", (int)fps), 0,0);
#endif
	/*if(bmCounter) {
		bmCounter--;
		if(!bmCounter) {
			glfwCloseWindow();
			winOpen = false;
			auto ms = chrono::duration_cast<chrono::microseconds>(t - benchStart).count();
			fprintf(stderr, "TIME: %ldus per frame\n", ms / 100);
		}
		return;
	}*/

	//GLSWAP

	auto ms = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	tween::Tween::updateTweens(ms / 1000000.0f);
	Resources::getInstance().update();
}

void Window::benchmark() {
	benchStart = chrono::high_resolution_clock::now();
	bmCounter = 100;
}

bool Window::mouse_pressed() {
	return false;
}

tuple<int, int> Window::mouse_position() {
	return make_tuple(-1,-1);
}

bool Window::key_pressed(key k) {
	return false;
}

bool Window::key_pressed(char k) {
	return false;
}

Window::click Window::get_click(bool peek) {
	return NO_CLICK;
}

Window::key Window::get_key(bool peek) {
	return NO_KEY;
};

Window screen;

}