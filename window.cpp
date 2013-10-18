#include "window.h"
#include "tween.h"

#include "GL_Header.h"
#include <GL/glfw.h>
#include <stdio.h>
#include <unordered_map>
//#include <math.h>

using namespace std;

void debug_callback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam) {
	LOGD("GLDEBUG:%s", message);
}

window::window() : basic_buffer(), winOpen(false), bmCounter(0) {
	NO_CLICK.x = NO_CLICK.y = NO_CLICK.button = -1;
}

void window::open(bool fs) {
	open(0,0,fs);
}

std::deque<int> window::key_buffer;
std::deque<window::click> window::click_buffer;

static void key_fn(int key, int action) {
	if(action == GLFW_PRESS)
		window::key_buffer.push_back(key);
}

static void mouse_fn(int button, int action) {
	if(action == GLFW_PRESS) {
		window::click c;
		glfwGetMousePos(&c.x, &c.y);
		c.button = button;
		window::click_buffer.push_back(c);
	}
}

void window::open(int w, int h, bool fs) {

	if(winOpen)
		return;

	glfwInit();
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 2);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
	GLFWvidmode mode;
	glfwGetDesktopMode(&mode);
	_width = w;
	_height = h;

	LOGD("Desktop is %dx%d", mode.Width, mode.Height);
	//mode.Width = 1600;

	if((float)mode.Width / (float)mode.Height > 2.2)
		mode.Width /= 2;

	if(_width <= 0) {
		_width = mode.Width;
		if(!fs)
			_width /= 2;
	}
	if(_height <= 0) {
		_height = mode.Height;
		if(!fs)
			_height /= 2;
	}


	int win = glfwOpenWindow(_width, _height, mode.RedBits, mode.GreenBits, mode.BlueBits, 0, 0, 0, fs ? GLFW_FULLSCREEN : GLFW_WINDOW);
	if(win) {
	}
	int rc = glewInit();
	if(rc) {
		LOGE("Glew error: %s", glewGetErrorString(rc));
		exit(0);
	}

	glDebugMessageCallbackARB(debug_callback, nullptr);

	glfwSwapInterval(1);

	glDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_ERROR_ARB, 1, 
             GL_DEBUG_SEVERITY_HIGH_ARB, 5, "YAY! ");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	lastTime = -1;
	winOpen = true;

	glfwSetKeyCallback(key_fn);
	glfwSetMouseButtonCallback(mouse_fn);
	/*[&](int key, int action) {
		if(action == GLFW_PRESS)
			key_buffer.push_back(key);
	});*/

	atexit([](){
		while(true) {
			//glfwSleep(100);
			glfwSwapBuffers();
			if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
				break;
		}	
	});

	startTime = chrono::high_resolution_clock::now();

	frameBuffer = 0;
};

void window::vsync() {
}

void window::flip() {
	auto t = chrono::high_resolution_clock::now();
	if(bmCounter) {
		bmCounter--;
		if(!bmCounter) {
			glfwCloseWindow();
			winOpen = false;
			auto ms = chrono::duration_cast<chrono::microseconds>(t - benchStart).count();
			fprintf(stderr, "TIME: %ldus per frame\n", ms / 100);
		}
		return;
	}
	glfwSwapBuffers();
	if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED)) {
		glfwCloseWindow();
		winOpen = false;			
	}

	auto ms = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	tween::Tween::updateTweens(ms / 1000000.0f);

}

void window::benchmark() {
	benchStart = chrono::high_resolution_clock::now();
	bmCounter = 100;
}

unordered_map<int, int> window::translate = {
	{ ENTER, GLFW_KEY_ENTER },
	{ LEFT, GLFW_KEY_LEFT },
	{ RIGHT, GLFW_KEY_RIGHT },
	{ UP, GLFW_KEY_UP },
	{ DOWN, GLFW_KEY_DOWN }
};

bool window::key_pressed(key k) {
	auto glfwKey = translate[k];
	return glfwGetKey(glfwKey) != 0;
}

window::click window::get_click() {
	if(click_buffer.size() > 0) {
		auto k = click_buffer.front();
		click_buffer.pop_front();
		return k;
	}
	return NO_CLICK;
}

window::key window::get_key() {
	if(key_buffer.size() > 0) {
		auto k = key_buffer.front();
		key_buffer.pop_front();
		for(auto t : translate) {
			if(t.second == k)
				return (key)t.first;
		}
	}
	return NO_KEY;
};

window screen;
