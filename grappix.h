#include "shader.h"

#include <GL/glew.h>

#include <coreutils/log.h>
#include <coreutils/utils.h>

#include <GL/glfw.h>
#include <stdio.h>
#include <math.h>

#include <chrono>
using namespace std;
using namespace utils;


template <typename T> T mapTo(T p, T view) {
	auto rp = p;
	rp[0] = (p[0] * 2.0 / view[0]) - 1.0;
	rp[1] = 1.0 - (p[1] * 2.0 / view[1]);
	return rp;
}

template <typename T = uint32_t> class basic_bitmap {
public:
	basic_bitmap(int width, int height) : w(width), h(height) {
		data.resize(width * height);
	}
	T& operator[](const int &i) { return data[i]; }
	int width() { return w; }
	int height() { return h; }
private:
	vector<T> data;
	int w;
	int h;
};

typedef basic_bitmap<uint32_t> bitmap;

class GLContext {
public:
	GLContext() {
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

		//programs[FLAT_SHADER] = createProgram(vShader, pShader);
		//programs[TEXTURED_SHADER] = createProgram(vTexShader, pTexShader);

		//glfwCloseWindow();
	}

	//enum {
	//	FLAT_SHADER,
	//	TEXTURED_SHADER
	//};

	//GLint getProgram(int what) {
	//	return get_program(what);
		//return programs[what];
	//}

	int getWidth() { return width; }
	int getHeight() { return height; }

private:
	//GLint programs[2];
	int width;
	int height;
};

GLContext &glcontext() {
	static GLContext context;
	return context;
}

/*
class texture {
public:
	template <typename BITMAP> texture(const BITMAP &bitmap) : id(-1) {

	}
private:
	GLint id;
};
*/


class basic_buffer {

public:

	basic_buffer() : frameBuffer(0), width(0), height(0), context(glcontext()) {}
	basic_buffer(GLint buffer, int width, int height) : frameBuffer(buffer), width(width), height(height), context(glcontext()) {
	}

	template <typename T> void line(T p0, T p1, uint32_t color) {
		line(p0[0], p0[1], p1[0], p1[1], color);
	}

	void line(const vector<int> &p0, const vector<int> &p1, uint32_t color) {
		line(p0[0], p0[1], p1[0], p1[1], color);
	}
	void clear() {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0,0,width,height);
		glClearColor(0.0, 0.0, 0.2, 0);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void line(float x0, float y0, float x1, float y1, uint32_t color) {

		//if(singleBuffer)
		//	glfwSwapBuffers();
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0,0,width,height);

		auto program = get_program(FLAT_PROGRAM);

		GLuint posHandle = glGetAttribLocation(program, "vPosition");
		GLuint colorHandle = glGetUniformLocation(program, "fColor");

		glUseProgram(program);

		glEnable(GL_POINT_SMOOTH);
		//glPointSize(radius);
		glLineWidth(14.0);

		float red = ((color>>16)&0xff) / 255.0;
		float green = ((color>>8)&0xff) / 255.0;
		float blue = (color&0xff) / 255.0;
		glUniform4f(colorHandle, red, green, blue, 1.0);

		float p[4];

		p[0] = (x0 * 2.0 / width) - 1.0;
		p[1] = 1.0 - (y0 * 2.0 / height);
		p[2] = (x1 * 2.0 / width) - 1.0;
		p[3] = 1.0 - (y1 * 2.0 / height);

		//glVertexAttrib2f(posHandle, p[0], p[1]);
		glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
		glEnableVertexAttribArray(posHandle);
		glDrawArrays(GL_LINES, 0, 2);
		//p[0] = p[1] = 0;
		//glDrawArrays(GL_LINE_STRIP, 0, points.size());
	
		//if(singleBuffer)
		//	glfwSwapBuffers();
	}

	template <typename T> void circle(T xy, float radius, uint32_t color) {
		circle(xy[0], xy[1], radius, color);
	}

	void circle(int x, int y, float radius, uint32_t color) {

		//if(singleBuffer)
		//	glfwSwapBuffers();
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0,0,width,height);

		auto program = get_program(FLAT_PROGRAM);

		GLuint posHandle = glGetAttribLocation(program, "vPosition");
		GLuint colorHandle = glGetUniformLocation(program, "fColor");

		glUseProgram(program);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_POINT_SMOOTH);
		glPointSize(radius);

		float red = ((color>>16)&0xff) / 255.0;
		float green = ((color>>8)&0xff) / 255.0;
		float blue = (color&0xff) / 255.0;
		glUniform4f(colorHandle, red, green, blue, 1.0);

		int count = radius;

		float p[count*2];


		for(int i=0; i<count; i++) {
			auto cx = cos(i*M_PI*2/count) * radius + x;
			auto cy = sin(i*M_PI*2/count) * radius + y;
			p[i*2] = (cx * 2.0 / width) - 1.0;
			p[i*2+1] = 1.0 - (cy * 2.0 / height);
		}

		LOGD("POS %f %f", p[0], p[1]);

		//glVertexAttrib2f(posHandle, p[0], p[1]);
		glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
		glEnableVertexAttribArray(posHandle);
		glDrawArrays(GL_TRIANGLE_FAN, 0, count);

		//if(singleBuffer)
		//	glfwSwapBuffers();
	}

	void draw(GLuint texture, int x0, int y0, int w, int h) {

		static float uvs[8] = {0,0, 1,0, 0,1, 1,1};


		//glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		//glViewport(0,0,width,height);
		//if(singleBuffer)
		//	glfwSwapBuffers();
		//glBindTexture(GL_TEXTURE_2D, texture);

		auto program = get_program(TEXTURED_PROGRAM);
		glUseProgram(program);

		GLuint posHandle = glGetAttribLocation(program, "vPosition");
		GLuint uvHandle = glGetAttribLocation(program, "vUV");
		//GLuint colorHandle = glGetUniformLocation(textureProgram, "fColor");

		float p[8];// = {-1,1, 1,1, -1,-1, 1,-1};

		p[0] = (x0 * 2.0 / width) - 1.0;
		p[1] = 1.0 - ((y0+h) * 2.0 / height);
		p[2] = ((x0+w) * 2.0 / width) - 1.0;
		p[3] = 1.0 - ((y0+h) * 2.0 / height);

		p[4] = (x0 * 2.0 / width) - 1.0;
		p[5] = 1.0 - (y0 * 2.0 / height);
		p[6] = ((x0+w) * 2.0 / width) - 1.0;
		p[7] = 1.0 - (y0 * 2.0 / height);

		glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
		glEnableVertexAttribArray(posHandle);
		glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 0, uvs);
		glEnableVertexAttribArray(uvHandle);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		//if(singleBuffer)
		//	glfwSwapBuffers();
	}

	int getWidth() { return width; }
	int getHeight() { return height; }

protected:
	GLuint frameBuffer;
	int width;
	int height;
	GLContext &context;
};

class renderbuffer : public basic_buffer {
public:

	template <typename T> renderbuffer(T size) : renderbuffer(size[0], size[1]) {	
	}

	renderbuffer(int width, int height) {
		// Create and bind a new framebuffer with emtpy attachment points (not yet useable)
		//GLuint fbuf;
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		// Create an empty texture with no filtering
		//GLuint renderedTexture;
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Bind the texture to the COLOR_ATTACHMENT of our framebuffer. This would not work on the default (0) framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);
		//glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
		//glBindFramebuffer(GL_FRAMEBUFFER, fbuf);

		this->width = width;
		this->height = height;

		/*
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		glfwSleep(100);
	}
*/

	}

	GLuint texture() { return texture_id; }

private:
	GLuint texture_id;
};

class window : public basic_buffer {
public:

	window() : basic_buffer(), winOpen(false), bmCounter(0) {
		open();
	}
	/*window(int width, int height) : basic_buffer(0, width, height), winOpen(false) {
		open();
	}*/

	void draw(renderbuffer &buffer, int x, int y) {
		basic_buffer::draw(buffer.texture(), x, y, buffer.getWidth(), buffer.getHeight());
	};

	template <typename T> void draw(T pos, renderbuffer &buffer) {
		basic_buffer::draw(buffer.texture(), pos[0], pos[1], buffer.getWidth(), buffer.getHeight());
	};

	void open() {

		if(winOpen)
			return;

		if(!width)
			width = context.getWidth();
		if(!height)
			height = context.getHeight();

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

	void flip() {
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

	bool is_open() { return winOpen; }

	vec2f size() { return vec2f(width, height); }

	constexpr static const double FPS = 1.0/60.0;

	void benchmark() {
		benchStart = chrono::high_resolution_clock::now();
		bmCounter = 100;
	}

private:
	int lastTime;
	bool winOpen;
	uint bmCounter;
	chrono::high_resolution_clock::time_point benchStart;
};

window screen;
