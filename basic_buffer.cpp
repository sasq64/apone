#include <coreutils/log.h>

#include "basic_buffer.h"
#include "shader.h"

#include "GL_Header.h"
#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>
using namespace std;
void basic_buffer::clear() {
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void basic_buffer::line(float x0, float y0, float x1, float y1, uint32_t color) {

	//if(singleBuffer)
	//	glfwSwapBuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto program = get_program(FLAT_PROGRAM);

	GLuint posHandle = glGetAttribLocation(program, "vPosition");
	GLuint colorHandle = glGetUniformLocation(program, "vColor");

	glUseProgram(program);

	//glEnable(GL_POINT_SMOOTH);
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

static GLint circleBuf = -1;

void basic_buffer::circle(int x, int y, float radius, uint32_t color) {

	//if(singleBuffer)
	//	glfwSwapBuffers();
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);

	auto program = get_program(FLAT_PROGRAM);

	GLuint posHandle = glGetAttribLocation(program, "vertex");
	GLuint colorHandle = glGetUniformLocation(program, "vColor");

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

	glUseProgram(program);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_POINT_SMOOTH);
	//glPointSize(radius);

	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	glUniform4f(colorHandle, red, green, blue, 1.0);


	//LOGD("POS %f %f", p[0], p[1]);
	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform2f(whHandle, 2.0 / _width, 2.0 / _height);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform2f(sHandle, radius * globalScale, radius * globalScale);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform2f(pHandle, x * globalScale, y * globalScale);

	//glVertexAttrib2f(posHandle, p[0], p[1]);
	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posHandle);
	glDrawArrays(GL_TRIANGLE_FAN, 0, count);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

GLint recBuf = -1;


void basic_buffer::draw_texture(GLint texture, float x, float y, float w, float h, float *uvs, GLint program) const {
	//static float suvs[8] = {0,1, 1,1, 0,0, 1,0};
	//static float suvs[8] = {0,0, 1,0, 0,1, 1,1};

	//if(!uvs)
	//	uvs = suvs;


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

	if(program < 0) {
		program = get_program(TEXTURED_PROGRAM);
	}
	glUseProgram(program);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	//if(singleBuffer)
	//	glfwSwapBuffers();
	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);


	GLuint posHandle = glGetAttribLocation(program, "vertex");
	GLuint uvHandle = glGetAttribLocation(program, "uv");
	//GLuint colorHandle = glGetUniformLocation(textureProgram, "fColor");

	vector<float> p {-1, 1, 1, 1, -1, -1, 1, -1};

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform4f(sHandle, globalScale * w/2, globalScale * h/2, 0, 1);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform4f(pHandle, x + w/2, (y + h/2), 0, 1);


	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 16, 0);
	glEnableVertexAttribArray(posHandle);
	glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
	glEnableVertexAttribArray(uvHandle);

	//glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, &p[0]);
	//glEnableVertexAttribArray(posHandle);
	//glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 0, uvs);
	//glEnableVertexAttribArray(uvHandle);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

GLint multiBuf[2] = {-1, -1};


void basic_buffer::draw_texture(GLint texture, float *points, int count, float w, float h, float *uvs, GLint program) const {

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


	if(program < 0) {
		program = get_program(TEXTURED_PROGRAM);
	}
	glUseProgram(program);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	//if(singleBuffer)
	//	glfwSwapBuffers();
	if(texture >= 0)
		glBindTexture(GL_TEXTURE_2D, texture);


	GLuint posHandle = glGetAttribLocation(program, "vertex");
	GLuint uvHandle = glGetAttribLocation(program, "uv");
	//GLuint colorHandle = glGetUniformLocation(textureProgram, "fColor");

	//vector<float> p {-1, 1, 1, 1, -1, -1, 1, -1};

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform4f(sHandle, globalScale, globalScale, 0, 1);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform4f(pHandle, 0, 0, 0, 1);

	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, (void*)(count*8*4));
	glEnableVertexAttribArray(posHandle);
	glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(uvHandle);

 	//static float uva[8] = {0.0,0.0, 1.0,0.0, 0.0,1.0, 1.0,1.0};
 	//GLuint uvsHandle = glGetUniformLocation(program, "uvs");
 	//glUniform1fv(uvsHandle, 8, uva);

	//glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, &p[0]);
	//glEnableVertexAttribArray(posHandle);
	//glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 0, uvs);
	//glEnableVertexAttribArray(uvHandle);

	//glDrawArrays(GL_TRIANGLES, 0, count*4);
	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(uvHandle);
	glDisableVertexAttribArray(posHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//if(singleBuffer)
	//	glfwSwapBuffers();
}

#if 0

void basic_buffer::draw_object(const gl_object &vbo, float x, float y, uint32_t color, float scale, float rotation) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, _width, _height);

	GLuint u;

	glUseProgram(vbo.program);

	u = glGetUniformLocation(vbo.program, "vScreenScale");
	glUniform2f(u, 2.0 / _width, 2.0 / _height);

	u = glGetUniformLocation(vbo.program, "vPosition");
	glUniform2f(u, x, y);

	u = glGetUniformLocation(vbo.program, "vScale");
	glUniform2f(u, globalScale * scale, globalScale * scale);

	u = glGetUniformLocation(vbo.program, "fRotation");
	glUniform1f(u, rotation);

	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	float alpha = ((color>>24)&0xff) / 255.0;

	u = glGetUniformLocation(vbo.program, "vColor");
	glUniform4f(u, red, green, blue, alpha);

	for(const auto &c : vbo.uniforms) {
		switch(c.type) {
		case 1:
			glUniform1f(c.handle, c.f[0]);
			break;
		case 2:
			glUniform2f(c.handle, c.f[0], c.f[1]);
			break;
		case 3:
			glUniform3f(c.handle, c.f[0], c.f[1], c.f[2]);
			break;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo.vertexBuf);
	if(vbo.iCount)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo.indexBuf);

	for(const auto &a : vbo.attributes) {
		uintptr_t p = a.offset;
		glVertexAttribPointer(a.handle, 2, GL_FLOAT, GL_FALSE, a.stride, (void*)p);
		glEnableVertexAttribArray(a.handle);
	}

	if(vbo.iCount)
		glDrawElements(vbo.primitive, vbo.iCount, GL_UNSIGNED_SHORT, 0);
	else
		glDrawArrays(vbo.primitive, 0, vbo.vCount);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

};


gl_object basic_buffer::make_rectangle(float w, float h) {
	gl_object obj;
	obj.program = get_program(FLAT_PROGRAM);
	w /= 2;
	h /= 2;
	vector<float> p {-w, h, w, h, -w, -h, w, -h};
	glGenBuffers(1, &obj.vertexBuf);
	glBindBuffer(GL_ARRAY_BUFFER, obj.vertexBuf);
	glBufferData(GL_ARRAY_BUFFER, p.size() * 4, &p[0], GL_STATIC_DRAW);
	obj.vCount = 4;
	obj.iCount = 0;
	obj.primitive = GL_TRIANGLE_STRIP;
	GLuint posHandle = glGetAttribLocation(obj.program, "vertex");
	obj.attributes.push_back(attribute(posHandle, 0, 0));

/*
	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	obj.uniforms.push_back(make_pair(whHandle, vec3f(2.0 / _width, 2.0 / _height, 0)));

	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint sHandle = glGetUniformLocation(program, "vScale");
	glUniform4f(sHandle, globalScale * w/2, globalScale * h/2, 0, 1);

	GLuint pHandle = glGetUniformLocation(program, "vPosition");
	glUniform4f(pHandle, x + w/2, y + h/2, 0, 1);
*/

	return obj;
};

#endif

void basic_buffer::rectangle(float x, float y, float w, float h, uint32_t color, float scale) {

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0,0,_width,_height);
	auto program = get_program(FLAT_PROGRAM);
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
	//if(singleBuffer)
	//	glfwSwapBuffers();
}

#ifdef WITH_FREETYPE

uint8_t *make_distance_map(uint8_t *img, int width, int height);

void basic_buffer::set_font(const string &ttfName, int size, int flags) {

	LOGD("Loading font");

	atlas = texture_atlas_new(256, 256, 1 );

	const wchar_t *text = L"@!ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ";
	font = texture_font_new(atlas, ttfName.c_str(), size);

	LOGD("Result %p", font);

	texture_font_load_glyphs(font, text);

	if(flags & DISTANCE_MAP) {
		uint8_t *data = make_distance_map(atlas->data, atlas->width, atlas->height);
		free(atlas->data);
		atlas->data = data;
	}
}

//static float scale = 1.0;

vector<GLuint> basic_buffer::make_text(const string &text) {

	LOGD("Make text");

	//auto tl = text.length();
	//vector<GLfloat> p;
	vector<GLfloat> verts;
	vector<GLushort> indexes;

	char lastChar = 0;
	int i = 0;
	float x = 0;
	float y = 0;
	for(auto c : text) {
		texture_glyph_t *glyph = texture_font_get_glyph(font, c);
		//LOGD("Glyph %p", glyph);
		//if( glyph == NULL )
		if(lastChar)
			x += texture_glyph_get_kerning(glyph, lastChar);
		lastChar = c;

		float x0  = x + glyph->offset_x;
		float y0  = y + glyph->offset_y;
		float x1  = x0 + glyph->width;
		float y1  = y0 - glyph->height;

		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		verts.push_back(x0);
		verts.push_back(y1);
		verts.push_back(s0);
		verts.push_back(t0);
		verts.push_back(x1);
		verts.push_back(y1);
		verts.push_back(s1);
		verts.push_back(t0);
		verts.push_back(x0);
		verts.push_back(y0);
		verts.push_back(s0);
		verts.push_back(t1);
		verts.push_back(x1);
		verts.push_back(y0);
		verts.push_back(s1);
		verts.push_back(t1);

		indexes.push_back(i);
		indexes.push_back(i+1);
		indexes.push_back(i+2);
		indexes.push_back(i+1);
		indexes.push_back(i+3);
		indexes.push_back(i+2);
		i += 4;

		x += glyph->advance_x;

		//break;
	}

	vector<GLuint> vbuf(2);
	glGenBuffers(2, &vbuf[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbuf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * 4, &verts[0], GL_STATIC_DRAW);

	return vbuf;
}

void basic_buffer::render_text(int x, int y, vector<GLuint> vbuf, int tl, uint32_t color, float scale) {

	//LOGD("[%f]", uvs);
	//auto _width = screen.size().first;
	//auto _height = screen.size().second;

	//LOGD("Render text %d %d", vbuf[0], vbuf[1]);

	glBindBuffer(GL_ARRAY_BUFFER, vbuf[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbuf[1]);


	auto program = get_program(FONT_PROGRAM_DF);
	//LOGD("Program %d", program);
	glUseProgram(program);

	GLuint vertHandle = glGetAttribLocation(program, "vertex");
	GLuint uvHandle = glGetAttribLocation(program, "uv");

	//uint32_t color = 0x40ff80;

	GLuint whHandle = glGetUniformLocation(program, "vScreenScale");
	glUniform4f(whHandle, 2.0 / _width, 2.0 / _height, 0, 1);

	GLuint posHandle = glGetUniformLocation(program, "vPosition");
	GLuint scaleHandle = glGetUniformLocation(program, "vScale");
	GLuint colorHandle = glGetUniformLocation(program, "vColor");
	float red = ((color>>16)&0xff) / 255.0;
	float green = ((color>>8)&0xff) / 255.0;
	float blue = (color&0xff) / 255.0;
	float alpha = ((color>>24)&0xff) / 255.0;
	glUniform4f(colorHandle, red, green, blue, alpha);
	glUniform4f(scaleHandle, scale, scale, 0, 1);
	glUniform4f(posHandle, x, y, 0, 1);
	//scale *= 1.001;


	glVertexAttribPointer(vertHandle, 2, GL_FLOAT, GL_FALSE, 16, 0);
	glEnableVertexAttribArray(vertHandle);
	glVertexAttribPointer(uvHandle, 2, GL_FLOAT, GL_FALSE, 16, (void*)8);
	glEnableVertexAttribArray(uvHandle);

	glBindTexture( GL_TEXTURE_2D, atlas->id );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glDrawElements(GL_TRIANGLES, 6*tl, GL_UNSIGNED_SHORT, 0);

	//LOGD("Drew %d\n", tl);

	glDisableVertexAttribArray(uvHandle);
	glDisableVertexAttribArray(vertHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


}

#include <unordered_map>

template <typename T, typename V> class VBLCache {
public:
	void put(const T &id, const V &value) {
		map[id] = value;
	}
	V get(const T &id) {
		return map[id];
	}
private:
	std::unordered_map<T, V> map;
};

VBLCache<std::string, std::vector<unsigned int>> cache;

void basic_buffer::text(int x, int y, const std::string &text, uint32_t col, float scale) {
	if(!atlas)
#ifdef ANDROID
		set_font("/sdcard/ObelixPro.ttf", 32);
#else
		set_font("fonts/ObelixPro.ttf", 32);
#endif

	auto buf = cache.get(text);
	if(buf.size() == 0) {
		buf = make_text(text);
		cache.put(text, buf);
	}
	render_text(x, y, buf, text.length(), col, scale);

	//LOGD("Deleting buffers %d %d", buf[0], buf[1]);
	//glDeleteBuffers(2, &buf[0]);

	//LOGD("Done");
}
 
#endif
