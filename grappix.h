#include "shader.h"
#include "texture.h"
#include "window.h"
#include "bitmap.h"
#include "image.h"
#include "tiles.h"

uint32_t blend(uint32_t col0, uint32_t col1, float alpha) {
	auto a0 = (col0>>24)&0xff;
	auto r0 = (col0>>16)&0xff;
	auto g0 = (col0>>8)&0xff;
	auto b0 = col0&0xff;

	auto a1 = (col1>>24)&0xff;
	auto r1 = (col1>>16)&0xff;
	auto g1 = (col1>>8)&0xff;
	auto b1 = col1&0xff;

	a0 = a0*alpha+a1*(1.0-alpha);
	r0 = r0*alpha+r1*(1.0-alpha);
	g0 = g0*alpha+g1*(1.0-alpha);
	b0 = b0*alpha+b1*(1.0-alpha);
	return (a0<<24) | (r0<<16) | (g0<<8) | b0;
}

/*
template <typename T> T mapTo(T p, T view) {
	auto rp = p;
	rp[0] = (p[0] * 2.0 / view[0]) - 1.0;
	rp[1] = 1.0 - (p[1] * 2.0 / view[1]);
	return rp;
}

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

class texture {
public:
	template <typename BITMAP> texture(const BITMAP &bitmap) : id(-1) {

	}
private:
	GLint id;
};
*/



