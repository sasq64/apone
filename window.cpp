#include "window.h"

#include <GL/glew.h>
#include <GL/glfw.h>
#include <stdio.h>
//#include <math.h>

using namespace std;

void window::open() {

	if(winOpen)
		return;

	glfwInit();
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
	glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 0);
	GLFWvidmode mode;
	glfwGetDesktopMode(&mode);
	width = mode.Width;// / 2;
	height = mode.Height;// / 2;
	int win = glfwOpenWindow(width, height, mode.RedBits, mode.GreenBits, mode.BlueBits, 0, 0, 0, GLFW_FULLSCREEN);
	if(win) {
	}
	int rc = glewInit();
	if(rc) {
		LOGE("Glew error: %s", glewGetErrorString(rc));
		exit(0);
	}
	glfwSwapInterval(1);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	lastTime = -1;
	winOpen = true;

	atexit([](){
		while(true) {
			//glfwSleep(100);
			glfwSwapBuffers();
			if(glfwGetKey(GLFW_KEY_ESC) || !glfwGetWindowParam(GLFW_OPENED))
				break;
		}	
	});

	frameBuffer = 0;
};

void window::flip() {
	if(bmCounter) {
		bmCounter--;
		if(!bmCounter) {
			glfwCloseWindow();
			winOpen = false;
			auto t = chrono::high_resolution_clock::now();
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
}

void window::benchmark() {
	benchStart = chrono::high_resolution_clock::now();
	bmCounter = 100;
}

window screen;
