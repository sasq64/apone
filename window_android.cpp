#include "window.h"

#include <stdio.h>
#include <unordered_map>
//#include <math.h>

using namespace std;

void debug_callback(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, void* userParam) {
	LOGD("GLDEBUG:%s", message);
}

void window::open(bool fs) {
	open(0,0,fs);
}

void window::open(int w, int h, bool fs) {

	if(winOpen)
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	lastTime = -1;
	winOpen = true;

	atexit([](){
		while(true) {
			//glfwSleep(100);
			//glfwSwapBuffers();
			//if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
			//	break;
		}	
	});

	frameBuffer = 0;
};

void window::vsync() {
}

void window::flip() {
	//glfwSwapBuffers();
	//if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED)) {
	//	glfwCloseWindow();
	//	winOpen = false;			
	//}
}

void window::benchmark() {
	benchStart = chrono::high_resolution_clock::now();
	bmCounter = 100;
}

/*
unordered_map<int, int> window::translate = {
	{ ENTER, GLFW_KEY_ENTER },
	{ LEFT, GLFW_KEY_LEFT },
	{ RIGHT, GLFW_KEY_RIGHT },
	{ UP, GLFW_KEY_UP },
	{ DOWN, GLFW_KEY_DOWN }
};*/

bool window::key_pressed(key k) {
	//auto glfwKey = translate[k];
	return false;
	//return glfwGetKey(glfwKey) != 0;
}

window screen;
