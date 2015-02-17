#include "GL_Header.h"


#include "render_target.h"
#include "shader.h"
#include "color.h"
#include "transform.h"
#include <coreutils/log.h>
#include <coreutils/mat.h>
#ifdef WITH_FREETYPE
#include <freetype-gl.h>
#endif

//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;
using namespace utils;

namespace grappix {

GLint RenderTarget::circleBuf = -1;
GLint RenderTarget::recBuf = -1;
GLint RenderTarget::lineBuf = -1;
static GLint lineBuf2 = -1;
GLint RenderTarget::multiBuf[2] = {-1, -1};

void RenderTarget::clear(uint32_t color) {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	auto c = Color(color);
	glClearColor(c.red, c.green, c.blue, c.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RenderTarget::glowLine(vec2f v0, vec2f v1, uint32_t color) {

	auto vn0 = (v1-v0).normalize();
	auto vn1 = (v0-v1).normalize();

	if(lineBuf2 == -1) {
		static vector<float> p {
			-1, 1,
			1, 1,
			-1, -1,
			1, -1,
			0,0,1,0,0,1,1,1
		};
		glGenBuffers(1, (GLuint*)&lineBuf2);
		glBindBuffer(GL_ARRAY_BUFFER, lineBuf2);
		glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, lineBuf2);
	}

 	vector<float> p(8);
	p[0] = v0.x - vn0.y;
	p[1] = v0.y + vn0.x;

	p[2] = v1.x - vn1.y;
	p[3] = v1.y + vn1.x;

	p[4] = v0.x + vn0.y;
	p[5] = v0.y - vn0.x;

	p[6] = v1.x + vn1.y;
	p[7] = v1.y - vn1.x;

	glBufferSubData(GL_ARRAY_BUFFER, 8 * 4, 0, &p[0]);

	auto &program = get_program(FLAT_PROGRAM);

	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	//if(texture >= 0)
	//	glBindTexture(GL_TEXTURE_2D, texture);

	mat4f matrix = make_scale(globalScale, globalScale);
	//matrix = make_rotate_z(xrot) * matrix;
	//xrot += 0.5;
	//matrix = make_translate(x + w/2, y + h/2) * matrix;

	//matrix = make_perspective(M_PI, 1.0, 0, 10) * matrix;

	matrix = toScreen * matrix;

	program.setUniform("matrix", matrix.transpose());

	auto c = Color(color);
	program.setUniform("color", c.red, c.green, c.blue, c.alpha);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 16, 0);
	//if(uvs)
	//	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, 16*4);
	//else
	//	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 16, 8);


	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//if(texture >= 0)
	//	glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTarget::line(float x0, float y0, float x1, float y1, uint32_t color) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto &program = get_program(FLAT_PROGRAM);
	program.use();

	if(lineBuf == -1) {
		glGenBuffers(1, (GLuint*)&lineBuf);
	}
	glBindBuffer(GL_ARRAY_BUFFER, lineBuf);

	float p[4] = { x0, y0, x1, y1 };
	glBufferData(GL_ARRAY_BUFFER, 4*4, p, GL_DYNAMIC_DRAW);

	GLuint posHandle = program.getAttribLocation("vertex");

	glLineWidth(4.0);

	auto c = Color(color);
	program.setUniform("color", c.red, c.green, c.blue, 1.0);


	mat4f matrix = make_scale(globalScale, globalScale);
	//matrix = make_translate(x + w/2, y + h/2) * matrix;
	matrix = toScreen * matrix;
	program.setUniform("matrix", matrix.transpose());


	program.setUniform("vScreenScale", 2.0f / _width, 2.0f / _height);
	program.setUniform("vScale", globalScale, globalScale);
	program.setUniform("vPosition", 0, 0);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_LINES, 0, 2);
}

void RenderTarget::dashed_line(float x0, float y0, float x1, float y1, uint32_t color) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto &program = get_program(FLAT_PROGRAM);
	program.use();

	if(lineBuf == -1) {
		glGenBuffers(1, (GLuint*)&lineBuf);
	}
	glBindBuffer(GL_ARRAY_BUFFER, lineBuf);

	GLuint posHandle = program.getAttribLocation("vertex");

	glLineWidth(2.0);

	auto c = Color(color);
	program.setUniform("color", c.red, c.green, c.blue, 1.0);

	program.setUniform("vScreenScale", 2.0 / _width, 2.0 / _height);
	program.setUniform("vScale", globalScale, globalScale);
	program.setUniform("vPosition", 0, 0);

	//float p[4] = { x0, y0, x1, y1 };
	float x = x0;
	float y = y0;

	int steps = 28;
	float dx = (x1-x0)/steps;
	float dy = (y1-y0)/steps;

	vector<float> points;
	points.reserve(steps*2);

	for(int i=0; i<steps/2; i++) {
		points.push_back(x);
		points.push_back(y);
		points.push_back(x+dx);
		points.push_back(y+dy);
		x += dx*2;
		y += dy*2;
	}
	glBufferData(GL_ARRAY_BUFFER, points.size()*4, &points[0], GL_DYNAMIC_DRAW);


	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_LINES, 0, steps);

}


void RenderTarget::circle(int x, int y, float radius, uint32_t color) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto &program = get_program(FLAT_PROGRAM);

	program.use();

	int count = 64;

	if(circleBuf == -1) {
		vector<float> p(count*2);
		for(int i=0; i<count; i++) {
			p[i*2] = cos(i*M_PI*2/count);
			p[i*2+1] = sin(i*M_PI*2/count);
		}
		glGenBuffers(1, (GLuint*)&circleBuf);
		glBindBuffer(GL_ARRAY_BUFFER, circleBuf);
		glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	} else
		glBindBuffer(GL_ARRAY_BUFFER, circleBuf);

	auto c = Color(color);
	program.setUniform("color", c.red, c.green, c.blue, 1.0);

	mat4f matrix = make_scale(globalScale * radius, globalScale * radius);
	matrix = make_translate(x * globalScale, y * globalScale) * matrix;
	matrix = toScreen * matrix;
	program.setUniform("matrix", matrix.transpose());

	GLuint posHandle = program.getAttribLocation("vertex");
	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(posHandle);
}

//static float xrot = 0;

void RenderTarget::draw_texture(GLint texture, float x, float y, float w, float h, float *uvs, const Program &program, uint32_t color) const {
//	static float uvs2[] = { 0,0,1,0,0,1,1,1 };

	//uvs = nullptr;//uvs2;


	if(recBuf == -1) {
		static vector<float> p {
			-1, 1, 0, 0,
			1, 1, 1, 0,
			-1, -1, 0, 1,
			1, -1, 1, 1,
			0,0,1,0,0,1,1,1
		};
		glGenBuffers(1, (GLuint*)&recBuf);
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
		glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
	}
	if(uvs)
		glBufferSubData(GL_ARRAY_BUFFER, 16 * 4, 8*4, uvs);

	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);

	float d = 1.0 - ((y+h) / (float)_height);
	if(d < 0) d = 0;
	if(d > 1) d = 1;
	program.setUniform("vUniformZ", d);

	mat4f matrix = make_scale(globalScale * w/2, globalScale * h/2);
	//matrix = make_rotate_z(xrot) * matrix;
	//xrot += 0.5;
	matrix = make_translate(x + w/2, y + h/2) * matrix;

	//matrix = make_perspective(M_PI, 1.0, 0, 10) * matrix;

	matrix = toScreen * matrix;

	program.setUniform("matrix", matrix.transpose());

	auto c = Color(color);
	program.setUniform("color", c.red, c.green, c.blue, c.alpha);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 16, 0);
	if(uvs)
		program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, 16*4);
	else
		program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 16, 8);


	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, 0);
}


void RenderTarget::draw_textures(GLint texture, float *points, int count, float w, float h, float *uvs, const Program &program) const {

	if(multiBuf[0] == -1) {
		glGenBuffers(2, (GLuint*)multiBuf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);
		vector<uint16_t> indexes;
		indexes.reserve(count*6);
		int i=0;
		for(int j=0; j<count; j++) {
			indexes.push_back(i);
			indexes.push_back(i+1);
			indexes.push_back(i+2);
			indexes.push_back(i+1);
			indexes.push_back(i+3);
			indexes.push_back(i+2);
			i += 4;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);


		vector<float> coords;
		coords.reserve(count*16);
		for(int j=0; j<count; j++) {
			coords.push_back(0);
			coords.push_back(0);
			coords.push_back(1);
			coords.push_back(0);
			coords.push_back(0);
			coords.push_back(1);
			coords.push_back(1);
			coords.push_back(1);
		}

		glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);
		glBufferData(GL_ARRAY_BUFFER, coords.size() * 4 * 2, &coords[0], GL_DYNAMIC_DRAW);
	} else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);

	vector<float> coords;
	coords.reserve(count*8);
	for(int j=0; j<count; j++) {
		float x = *points++;
		float y = *points++;
		coords.push_back(x);
		coords.push_back(y+h);
		coords.push_back(x+w);
		coords.push_back(y+h);
		coords.push_back(x);
		coords.push_back(y);
		coords.push_back(x+w);
		coords.push_back(y);
	}
	glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);

	glBufferSubData(GL_ARRAY_BUFFER, count*8*4, coords.size() * 4, &coords[0]);

	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);
	//mat4f m = make_rotate_x(2.0);
	program.setUniform("matrix", toScreen.transpose());

	auto c = Color(0xffffffff);
	program.setUniform("color", c.red, c.green, c.blue, c.alpha);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 0, count*8*4);
	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	//glDisableVertexAttribArray(uvHandle);
	//glDisableVertexAttribArray(posHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void RenderTarget::rectangle(float x, float y, float w, float h, uint32_t color, const Program &program) {

	//float scale = 1.0;
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	//auto &program = get_program(FLAT_PROGRAM);
	program.use();

	if(recBuf == -1) {
		static vector<float> p {
			-1, 1, 0, 0,
			1, 1, 1, 0,
			-1, -1, 0, 1,
			1, -1, 1, 1,
			0,0,1,0,0,1,1,1
		};
		glGenBuffers(1, (GLuint*)&recBuf);
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
		glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
	}

	auto c = Color(color);
	program.setUniform("color", c.red, c.green, c.blue, c.alpha);

	mat4f matrix = make_scale(globalScale * w/2, globalScale * h/2);
	matrix = make_translate(x + w/2, y + h/2) * matrix;
	matrix = toScreen * matrix;

	program.setUniform("matrix", matrix.transpose());


	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 16, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderTarget::text(const Font &font, const std::string &text, float x, float y, uint32_t col, float scale) const {
	font.render_text(*this, text, x, y, col, scale);
}

void RenderTarget::text(const std::string &text, float x, float y, uint32_t col, float scale) const {

	if(!font)
		font = make_shared<Font>(true);

	font->render_text(*this, text, x, y, col, scale);
}




}