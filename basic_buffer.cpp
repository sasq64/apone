
#include "basic_buffer.h"
#include "shader.h"

#include <GL/glew.h>
#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;
void basic_buffer::clear() {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void basic_buffer::line(float x0, float y0, float x1, float y1, uint32_t color) {

	//if(singleBuffer)
	//	glfwSwapBuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

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

	p[0] = x0;//(x0 * 2.0 / _width) - 1.0;
	p[1] = y0;//1.0 - (y0 * 2.0 / _height);
	p[2] = x1;//(x1 * 2.0 / _width) - 1.0;
	p[3] = y1;//1.0 - (y1 * 2.0 / _height);

	//glVertexAttrib2f(posHandle, p[0], p[1]);
	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform2f(whHandle, 2.0 / _width, 2.0 / _height);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_LINES, 0, 2);
	//p[0] = p[1] = 0;
	//glDrawArrays(GL_LINE_STRIP, 0, points.size());

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

void basic_buffer::circle(int x, int y, float radius, uint32_t color) {

	//if(singleBuffer)
	//	glfwSwapBuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto program = get_program(FLAT_PROGRAM);

	GLuint posHandle = glGetAttribLocation(program, "vertex");
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
		p[i*2] = cos(i*M_PI*2/count);
		p[i*2+1] = sin(i*M_PI*2/count);
	}

	//LOGD("POS %f %f", p[0], p[1]);
	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform4f(sHandle, radius, radius, 0, 1);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform4f(pHandle, x, y, 0, 1);

	//glVertexAttrib2f(posHandle, p[0], p[1]);
	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

void basic_buffer::draw_texture(GLint texture, int x, int y, int w, int h) {

	static float uvs[8] = {0,1, 1,1, 0,0, 1,0};


	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	//if(singleBuffer)
	//	glfwSwapBuffers();
	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);

	auto program = get_program(TEXTURED_PROGRAM);
	glUseProgram(program);

	GLuint posHandle = glGetAttribLocation(program, "vertex");
	GLuint uvHandle = glGetAttribLocation(program, "uv");
	//GLuint colorHandle = glGetUniformLocation(textureProgram, "fColor");

	vector<float> p {-1, 1, 1, 1, -1, -1, 1, -1};

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform4f(sHandle, globalScale * w/2, globalScale * h/2, 0, 1);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform4f(pHandle, x + w/2, y + h/2, 0, 1);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, &p[0]);
	glEnableVertexAttribArray(posHandle);
	glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 0, uvs);
	glEnableVertexAttribArray(uvHandle);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

void basic_buffer::rectangle(float x, float y, float w, float h, uint32_t color) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	auto program = get_program(FLAT_PROGRAM);
	glUseProgram(program);

	GLuint posHandle = glGetAttribLocation(program, "vertex");
	GLuint colorHandle = glGetUniformLocation(program, "fColor");

	//vector<float> p {x, y+h, x+w, y+h, x, y, x+w, y};
	vector<float> p {-1, 1, 1, 1, -1, -1, 1, -1};

	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	glUniform4f(colorHandle, red, green, blue, 1.0);

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform4f(sHandle, globalScale * w/2, globalScale * h/2, 0, 1);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform4f(pHandle, x + w/2, y + h/2, 0, 1);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, &p[0]);
	glEnableVertexAttribArray(posHandle);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}
