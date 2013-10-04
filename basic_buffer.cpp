
#include "basic_buffer.h"
#include "shader.h"

#include <GL/glew.h>
#define _USE_MATH_DEFINES
#include <cmath>

void basic_buffer::clear() {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,width,height);
	glClearColor(0.0, 0.0, 0.2, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void basic_buffer::line(float x0, float y0, float x1, float y1, uint32_t color) {

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

void basic_buffer::circle(int x, int y, float radius, uint32_t color) {

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

	//LOGD("POS %f %f", p[0], p[1]);

	//glVertexAttrib2f(posHandle, p[0], p[1]);
	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

void basic_buffer::draw(GLuint texture, int x0, int y0, int w, int h) {

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
