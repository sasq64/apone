#include "GL_Header.h"


#include "render_target.h"
#include "shader.h"
#include "color.h"

//#include "obelix.h"

#include <coreutils/log.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;

namespace grappix {

GLint RenderTarget::circleBuf = -1;
GLint RenderTarget::recBuf = -1;
GLint RenderTarget::multiBuf[2] = {-1, -1};
//Program RenderTarget::NO_PROGRAM(-1);

void RenderTarget::clear(uint32_t color) {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	auto c = make_color(color);
	glClearColor(c.red, c.green, c.blue, c.alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void RenderTarget::line(float x0, float y0, float x1, float y1, uint32_t color) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto &program = get_program(FLAT_PROGRAM);

	GLuint posHandle = program.getAttribLocation("vPosition");
	GLuint colorHandle = program.getUniformLocation("vColor");

	program.use();

	glLineWidth(14.0);

	auto c = make_color(color);
	glUniform4f(colorHandle, c.red, c.green, c.blue, 1.0);

	float p[4];

	p[0] = x0;//(x0 * 2.0 / _width) - 1.0;
	p[1] = y0;//1.0 - (y0 * 2.0 / _height);
	p[2] = x1;//(x1 * 2.0 / _width) - 1.0;
	p[3] = y1;//1.0 - (y1 * 2.0 / _height);

	//glVertexAttrib2f(posHandle, p[0], p[1]);
	GLuint whHandle = program.getUniformLocation("vScreenScale");
	glUniform2f(whHandle, 2.0 / _width, 2.0 / _height);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, p);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_LINES, 0, 2);
	//p[0] = p[1] = 0;
	//glDrawArrays(GL_LINE_STRIP, 0, points.size());
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

	auto c = make_color(color);
	program.setUniform("vColor", c.red, c.green, c.blue, 1.0);

	program.setUniform("vScreenScale", 2.0 / _width, 2.0 / _height);
	program.setUniform("vScale", radius * globalScale, radius * globalScale);
	program.setUniform("vPosition", x * globalScale, y * globalScale);


	GLuint posHandle = program.getAttribLocation("vertex");
	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderTarget::draw_texture(GLint texture, float x, float y, float w, float h, float *uvs, Program &program) const {
	//static float suvs[8] = {0,1, 1,1, 0,0, 1,0};
	//static float suvs[8] = {0,0, 1,0, 0,1, 1,1};

	if(recBuf == -1) {
		static vector<float> p {
			-1, 1, 0, 0,
			1, 1, 1, 0,
			-1, -1, 0, 1,
			1, -1, 1, 1,
			0,0,0,0,0,0,0,0
		};
		glGenBuffers(1, (GLuint*)&recBuf);
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, recBuf+1);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
	}
	if(uvs)
		glBufferSubData(GL_ARRAY_BUFFER, 16 * 4, 8*4, uvs);

	//LOGD("Prog %p %d", &program, program.id());
	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);

	program.setUniform("vScreenScale", 2.0 / _width, 2.0 / _height, 0, 1);
	program.setUniform("vScale", globalScale * w/2, globalScale * h/2, 0, 1);
	program.setUniform("vPosition", x + w/2, (y + h/2), 0, 1);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 16, 0);
	if(uvs)
		program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, 16*4);
	else
		program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 16, 8);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	//LOGD("yy %d", program.attributes.size());
}

void RenderTarget::draw_texture(GLint texture, float *points, int count, float w, float h, float *uvs, Program &program) const {

	if(multiBuf[0] == -1) {
		glGenBuffers(2, (GLuint*)multiBuf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);
		vector<uint16_t> indexes;//(count*6);
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


		vector<float> coords;//(count*16);
		coords.reserve(count*8);
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

	vector<float> coords;//(count*16);
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
	//glBufferData(GL_ARRAY_BUFFER, coords.size() * 4, &coords[0], GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, count*8*4, coords.size() * 4, &coords[0]);

	//if(program == NO_PROGRAM)
	//program = get_program(TEXTURED_PROGRAM);

	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);



	program.setUniform("vScreenScale", 2.0 / _width, 2.0 / _height, 0, 1);
	program.setUniform("vScale", globalScale, globalScale, 0, 1);
	program.setUniform("vPosition", 0, 0, 0, 1);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 0, count*8*4);
	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	//glDisableVertexAttribArray(uvHandle);
	//glDisableVertexAttribArray(posHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void RenderTarget::rectangle(float x, float y, float w, float h, uint32_t color, float scale) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	auto program = get_program(FLAT_PROGRAM).id();
	glUseProgram(program);

	GLuint posHandle = glGetAttribLocation(program, "vertex");
	GLuint colorHandle = glGetUniformLocation(program, "vColor");

	//vector<float> p {x, y+h, x+w, y+h, x, y, x+w, y};

	if(recBuf == -1) {
		vector<float> p {
			-1, 1, 0, 0,
			1, 1, 1, 0,
			-1, -1, 0, 1,
			1, -1, 1, 1,
		};
		glGenBuffers(1, (GLuint*)&recBuf);
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, recBuf+1);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	} else
		glBindBuffer(GL_ARRAY_BUFFER, recBuf);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, recBuf+1);

	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	glUniform4f(colorHandle, red, green, blue, 1.0);

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform2f(whHandle, 2.0 / _width, 2.0 / _height);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform2f(sHandle, scale * globalScale * w/2, scale * globalScale * h/2);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform2f(pHandle, (x + w/2) * globalScale, ((y + h/2)) * globalScale);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 16, 0);
	glEnableVertexAttribArray(posHandle);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RenderTarget::text(const Font &font, const std::string &text, int x, int y, uint32_t col, float scale) const {
	font.render_text(*this, text, x, y, col, scale);
}

void RenderTarget::text(const std::string &text, int x, int y, uint32_t col, float scale) const {

	if(!font)
		font = make_shared<Font>(true);//"data/Vera.ttf", 16);

	font->render_text(*this, text, x, y, col, scale);
}

}