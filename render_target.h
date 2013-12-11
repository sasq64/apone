#ifndef GRAPPIX_BASIC_BUFFER_H
#define GRAPPIX_BASIC_BUFFER_H

#include "GL_Header.h"

#include "shader.h"
#include "font.h"

#ifdef WITH_FREETYPE
#include <freetype-gl.h>
#endif

#include <vector>
#include <string>
#include <stdint.h>
#include <memory>


/*
struct attribute {
	attribute(GLuint handle, uint16_t offset, uint16_t stride) : handle(handle), offset(offset), stride(stride) {}
	GLuint handle;
	uint16_t offset;
	uint16_t stride;
};

struct uniform {
	uniform(GLuint handle, float af) : handle(handle), type(1), f{af,0,0,0} {}
	GLuint handle;
	int type;
	float f[4];
};

struct gl_object {
	int primitive;
	GLuint vertexBuf;
	GLuint indexBuf;
	short iCount;
	short vCount;
	GLuint texture;

	GLuint program;

	std::vector<attribute> attributes;
	std::vector<uniform> uniforms;

}; */

class RenderTarget {
public:


	RenderTarget(bool fromWindow) : frameBuffer(0), _width(0), _height(0), globalScale(1.0)  {
	}

	RenderTarget() : frameBuffer(0), _width(0), _height(0), globalScale(1.0), 
		flatProgram { get_program(FLAT_PROGRAM) },
		texturedProgram { get_program(TEXTURED_PROGRAM) } {		
	}

	void initPrograms() {
		flatProgram = get_program(FLAT_PROGRAM);
		texturedProgram = get_program(TEXTURED_PROGRAM);
	}
/*
	RenderTarget(unsigned int buffer, int width, int height) : frameBuffer(buffer), _width(width), _height(height), globalScale(1.0), font(nullptr), atlas(nullptr),
	flatProgram { get_program_obj(FLAT_PROGRAM) },
	texturedProgram { get_program_obj(TEXTURED_PROGRAM) } {		
	}
*/
	~RenderTarget() {
	}

	//void draw_object(const gl_object &vbo, float x, float y, uint32_t color = 0xffffffff, float scale = 1.0f, float rotation = 0.0f);
	//gl_object make_rectangle(float w, float h);

	template <typename T> void line(T p0, T p1, uint32_t color) {
		line(p0[0], p0[1], p1[0], p1[1], color);
	}

	void line(const std::vector<int> &p0, const std::vector<int> &p1, uint32_t color) {
		line(p0[0], p0[1], p1[0], p1[1], color);
	}
	void clear(uint32_t color = 0);
	void line(float x0, float y0, float x1, float y1, uint32_t color);

	template <typename T> void circle(T xy, float radius, uint32_t color) {
		circle(xy[0], xy[1], radius, color);
	}

	template <typename T> void rectangle(T pos, T size, uint32_t color, float scale = 1.0) {
		rectangle(pos[0], pos[1], size[0], size[1], color, scale);
	}

	template <typename T> void rectangle(T pos, int w, int h, uint32_t color, float scale = 1.0) {
		rectangle(pos[0], pos[1], w, h, color, scale);
	}

	void rectangle(float x, float y, float w, float h, uint32_t color, float scale = 1.0);

	void circle(int x, int y, float radius, uint32_t color);

	template <typename T, typename V> void draw(const T &t, const V &pos) const {
		draw_texture(t.id(), pos[0], pos[1], t.width(), t.height(), nullptr, NO_PROGRAM);
	}

	template <typename T, typename V> void draw(const T &t, const V &pos, float w, float h, Program &program = NO_PROGRAM) const {
		draw_texture(t.id(), pos[0], pos[1], w, h, nullptr, program);
	}

	template <typename T> void draw(const T &t, float x0, float y0, float w, float h, Program &program = NO_PROGRAM) const {
		draw_texture(t.id(), x0, y0, w, h, nullptr, program);
	}

	template <typename T> void draw(const T &t, float x0, float y0) const {
		draw_texture(t.id(), x0, y0, t.width(), t.height(), nullptr, NO_PROGRAM);
	}

	void draw_texture(GLint texture, float *points, int count, float w, float h, float *uvs = nullptr, Program &program = NO_PROGRAM) const;
	void draw_texture(GLint texture, float x0, float y0, float w, float h, float *uvs = nullptr, Program &program = NO_PROGRAM) const;
	int width() const { return _width; }
	int height() const { return _height; }
	GLuint buffer() const { return frameBuffer; }

	float scale() const { return globalScale; }
	float scale(float s) { globalScale = s; return s; }

	void text(const std::string &text, int x = 0, int y = 0, uint32_t color = 0xffffffff, float scale = 1.0);
	void text(Font &font, const std::string &text, int x = 0, int y = 0, uint32_t color = 0xffffffff, float scale = 1.0);

	static Program NO_PROGRAM;

protected:
	unsigned int frameBuffer;
	int _width;
	int _height;
	float globalScale;

	std::shared_ptr<Font> font;

	Program flatProgram;
	Program texturedProgram;

	static GLint circleBuf;
	static GLint recBuf;
	static GLint multiBuf[2];
};

#endif // GRAPPIX_BASIC_BUFFER_H
