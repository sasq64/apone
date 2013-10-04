#include "shader.h"
#include "renderbuffer.h"
#include "window.h"
#include "bitmap.h"

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



