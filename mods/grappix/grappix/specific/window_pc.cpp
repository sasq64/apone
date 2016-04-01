#include "../window.h"
#include "../GL_Header.h"
#include <tween/tween.h>
#include <stdio.h>
#include <unordered_map>
#include <functional>
#include <GLFW/glfw3.h>

//#include <math.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace std;
	
namespace grappix {
void debug_callback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam) {
	LOGD("GLDEBUG:%s", message);
}

static GLFWwindow *gwindow;

Window::Window() : RenderTarget(), winOpen(false), bmCounter(0), lockIt(false) {
	NO_CLICK.x = NO_CLICK.y = NO_CLICK.button = -1;
}

Window::~Window() {
}

void Window::open(bool fs) {
	open(0,0,fs);
}

Window::click Window::NO_CLICK = { -1, -1, -1};
Window::Scroll Window::NO_SCROLL = { -9999, -9999};

std::deque<Window::click> Window::click_buffer;
std::deque<Window::Scroll> Window::scroll_buffer;

static void scroll_fn(GLFWwindow *gwin, double x, double y) {
	LOGD("SCROLL %f, %f", x, y);
	Window::scroll_buffer.push_back(Window::Scroll(x,y));
}


static void key_fn(GLFWwindow *gwin, int key, int scancode, int action, int mods) {
	if(action == GLFW_PRESS || action == GLFW_REPEAT) {

		int realkey = key;
		for(auto t : Window::translate) {
			if(t.second == key) {
				realkey = t.first;
				break;
			}
		}

		putEvent<KeyEvent>(realkey);
	}
		//Window::key_buffer.push_back(key);
}

static void mouse_fn(GLFWwindow *gwin, int button, int action, int mods) {
	if(action == GLFW_PRESS) {
		Window::click c;
		double x,y;
		glfwGetCursorPos(gwin, &x, &y);
		c.button = button;
		c.x = x;
		c.y = y;
		Window::click_buffer.push_back(c);
	}
}

#ifndef EMSCRIPTEN
static void resize_fn(GLFWwindow *gwin, int w, int h) {
	LOGD("Size now %d %d", w, h);

	//screen.resize(w, h);

	int fw, fh;
	glfwGetFramebufferSize(gwindow, &fw, &fh);
	glViewport(0, 0, fw, fh);
	LOGD("FB %d %d", fw, fh);
	screen.resize(fw, fh);

};
#endif

static function<void(uint32_t)> renderLoopFunction;

static void glfw_error(int code, const char *text) {
	LOGE("GLFW: %s (%d)", text, code);
}


void Window::open(int w, int h, bool fs) {

	if(winOpen)
		return;

	glfwSetErrorCallback(glfw_error);


	if(!glfwInit()) {
		LOGE("Could not initialize glfw");
		exit(EXIT_FAILURE);
	}
#ifdef EMSCRIPTEN
	if(w <= 0) w = 640;
	if(h <= 0) h = 480;
	_width = w;
	_height = h;
	fs = false;
	GLFWvidmode mode;
	mode.redBits = mode.greenBits = mode.blueBits = 8;
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
#else
	_width = w;
	_height = h;
	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	/*int monitorCount;
	GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
	LOGD("%d monitors", monitorCount);
	for(int i=0; i<monitorCount; i++) {
		int mw,mh;
		glfwGetMonitorPhysicalSize(monitors[i], &mw,  &mh);
		LOGD("%d: %dx%d", i, mw, mh);
	} */
	//GLFWmonitor *monitor = monitors[1];//glfwGetPrimaryMonitor();
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = nullptr;
	if(monitor)  
		mode = glfwGetVideoMode(monitor);
	else {
		LOGW("Could not get primary monitor");
		fs = false;
	}
	int modew = 640;
	int modeh = 480;
	if(mode) {
		modew = mode->width;
		modeh = mode->height;
	}


	//LOGD("Desktop is %dx%d", mode->width, mode->height);
	//mode->width = 1600;

	//if((float)mode->width / (float)mode.Height > 2.2)
	//	mode->width /= 2;

	if(_width <= 0) {
		_width = modew;
		if(!fs)
			_width /= 2;
	}
	if(_height <= 0) {
		_height = modeh;
		if(!fs)
			_height /= 2;
	}
#endif

	//_width *= 2;
	//_height *= 2;

	//gwindow = glfwOpenWindow(_width, _height, mode.RedBits, mode.GreenBits, mode.BlueBits, 8, 8, 0, fs ? GLFW_FULLSCREEN : GLFW_WINDOW);
	gwindow = glfwCreateWindow(_width, _height, "", fs ? monitor : nullptr, nullptr);
	glfwMakeContextCurrent(gwindow);
	//LOGD("%p WH %d %d", gwindow, _width, _height);

#ifndef EMSCRIPTEN
	int rc = glewInit();
	if(rc) {
		LOGE("Glew error: %s", glewGetErrorString(rc));
		exit(0);
	}
  #ifdef GL_DEBUG
	glDebugMessageCallbackARB(debug_callback, nullptr);
  #endif
#endif

	int fw, fh;
	glfwGetFramebufferSize(gwindow, &fw, &fh);
	glViewport(0, 0, fw, fh);
	LOGD("Window Size %dx%d -- Framebuffer Size  %dx%d", _width, _height, fw, fh);
	_width = fw;
	_height = fh;

	setup(_width, _height);

	//glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_ERROR_ARB, 1,
     //        GL_DEBUG_SEVERITY_HIGH_ARB, 5, "YAY! ");

	glfwSetKeyCallback(gwindow, key_fn);
	glfwSetMouseButtonCallback(gwindow, mouse_fn);
	glfwSetScrollCallback(gwindow, scroll_fn);
#ifndef EMSCRIPTEN
	glfwSetWindowSizeCallback(gwindow, resize_fn);
	glfwSwapInterval(1);
#endif
	//glfwEnable(GLFW_MOUSE_CURSOR);
	glfwSetInputMode(gwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	atexit([](){
		if(!renderLoopFunction) {
			while(screen.is_open()) {
				screen.update_callbacks();
				screen.flip();
			}
		}
	});

	flip();

};

#ifdef EMSCRIPTEN
static uint64_t lastMs = 0;
static void runMainLoop() {
	screen.update_callbacks();
	auto ms = utils::getms();
	uint32_t rate = ms - lastMs;
	lastMs = ms;
	if(renderLoopFunction)
		renderLoopFunction(rate);
}
#endif

void Window::render_loop(function<void(uint32_t)> f, int fps) {
	renderLoopFunction = f;
#ifdef EMSCRIPTEN
	emscripten_set_main_loop(runMainLoop, 0, false);
	glfwSwapInterval(1);
#else
	//atexit([](){
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
	//});

#endif
}

void Window::vsync() {
}

//static uint64_t lastTime;

void Window::close() {
	LOGD("QUIT");
	glfwDestroyWindow(gwindow);
	gwindow = nullptr;
	winOpen = false;
}

void Window::flip() {

	//auto t = chrono::high_resolution_clock::now();
	auto tm = utils::getms();
	auto d = tm - lastTime;
	lastTime = tm;

	if(d < 8)
		utils::sleepms(8);

#ifdef FPS_COUNTER
	if(d > 0)
		fps = fps * 0.8 + (1000 / d) * 0.2;
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

	if(!gwindow)
		return;

	glfwSwapBuffers(gwindow);
	glfwPollEvents();
	if(glfwWindowShouldClose(gwindow)) {
		close();
	}

	//auto ms = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	//tween::Tween::updateTweens(ms / 1000000.0f);
	//Resources::getInstance().update();

#ifdef EMSCRIPTEN
	int fs;
	emscripten_get_canvas_size(&_width, &_height, &fs);
#endif
}

unordered_map<int, int> Window::translate = {
	{ F1, GLFW_KEY_F1 },
	{ F2, GLFW_KEY_F2 },
	{ F3, GLFW_KEY_F3 },
	{ F4, GLFW_KEY_F4 },
	{ F5, GLFW_KEY_F5 },
	{ F6, GLFW_KEY_F6 },
	{ F7, GLFW_KEY_F7 },
	{ F8, GLFW_KEY_F8 },
	{ F9, GLFW_KEY_F9 },
	{ F10, GLFW_KEY_F10 },
	{ F11, GLFW_KEY_F11},
	{ F12, GLFW_KEY_F12 },
	{ BACKSPACE, GLFW_KEY_BACKSPACE },
	{ DELETE, GLFW_KEY_DELETE },
	{ ENTER, GLFW_KEY_ENTER },
	{ ESCAPE, GLFW_KEY_ESCAPE },
	{ SPACE, GLFW_KEY_SPACE },
	{ LEFT, GLFW_KEY_LEFT },
	{ TAB, GLFW_KEY_TAB },
	{ CTRL_LEFT, GLFW_KEY_LEFT_CONTROL },
	{ CTRL_RIGHT, GLFW_KEY_RIGHT_CONTROL },
	{ ALT_LEFT, GLFW_KEY_LEFT_ALT },
	{ ALT_RIGHT, GLFW_KEY_RIGHT_ALT },
	{ SHIFT_LEFT, GLFW_KEY_LEFT_SHIFT },
	{ SHIFT_RIGHT, GLFW_KEY_RIGHT_SHIFT },
	{ RIGHT, GLFW_KEY_RIGHT },
	{ UP, GLFW_KEY_UP },
	{ DOWN, GLFW_KEY_DOWN },
	{ PAGEUP, GLFW_KEY_PAGE_UP },
	{ PAGEDOWN, GLFW_KEY_PAGE_DOWN }
};

bool Window::mouse_pressed() {
	return glfwGetMouseButton(gwindow, 0);
}

tuple<int, int> Window::mouse_position() {
	double x,y;
	glfwGetCursorPos(gwindow, &x, &y);
	return make_tuple((int)x, (int)y);
}

bool Window::key_pressed(key k) {
	auto glfwKey = translate[k];
	return glfwGetKey(gwindow, glfwKey) != 0;
}

bool Window::key_pressed(char k) {
	return glfwGetKey(gwindow, k) != 0;
}

Window::click Window::get_click(bool peek) {
	if(click_buffer.size() > 0) {
		auto k = click_buffer.front();
		if(!peek)
			click_buffer.pop_front();
		return k;
	}
	return NO_CLICK;
}

Window::Scroll Window::get_scroll(bool peek) {
	if(scroll_buffer.size() > 0) {
		auto k = scroll_buffer.front();
		if(!peek)
			scroll_buffer.pop_front();
		return k;
	}
	return NO_SCROLL;
}

//Window::click Window::get_click(bool peek) {


Window::key Window::get_key(bool peek) {
	if(hasEvents<KeyEvent>()) {
		auto ke = getEvent<KeyEvent>();
		auto k = ke.code;
		return (key)k;
	}
	return NO_KEY;
};

//Window screen;

std::shared_ptr<Window> screenptr = make_shared<Window>();
Window& screen = *screenptr;

}
