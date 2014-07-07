#ifndef GRAPPIX_BASIC_BUFFER_H
#define GRAPPIX_BASIC_BUFFER_H

#include "GL_Header.h"

#include "shader.h"
#include "font.h"
#include "transform.h"
#include "rectangle.h"
#include <image/bitmap.h>

#include <vector>
#include <string>
#include <stdint.h>
#include <memory>
#include <type_traits>

namespace grappix {

class RenderTarget {
public:

	RenderTarget() : frameBuffer(0), _width(0), _height(0), globalScale(1.0) {}
	~RenderTarget() {}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void line(T p0, T p1, uint32_t color) {
		line(p0[0], p0[1], p1[0], p1[1], color);
	}
	void line(float x0, float y0, float x1, float y1, uint32_t color);
	void dashed_line(float x0, float y0, float x1, float y1, uint32_t color);

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void circle(T xy, float radius, uint32_t color) {
		circle(xy[0], xy[1], radius, color);
	}
	void circle(int x, int y, float radius, uint32_t color);
/*
	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void rectangle(T pos, T size, uint32_t color, float scale = 1.0) {
		rectangle(pos[0], pos[1], size[0], size[1], color, scale);
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void rectangle(T pos, int w, int h, uint32_t color, float scale = 1.0) {
		rectangle(pos[0], pos[1], w, h, color, scale);
	}
*/
	void rectangle(const Rectangle &rec, uint32_t color, const Program &program = get_program(FLAT_PROGRAM)) {
		rectangle(rec.x, rec.y, rec.w, rec.h, color, program);
	}
	void rectangle(const Rectangle &rec, const Program &program = get_program(FLAT_PROGRAM)) {
		rectangle(rec.x, rec.y, rec.w, rec.h, 0xffffffff, program);
	}
	void rectangle(float x, float y, float w, float h, uint32_t color, const Program &program = get_program(FLAT_PROGRAM));

	template <typename TEXTURE, class = typename std::enable_if<std::is_compound<TEXTURE>::value>::type>
	void image(TEXTURE t, float x, float y, const Program &program = get_program(TEXTURED_PROGRAM) ) const {
		draw_texture(t.id(), x, y, t.width(), t.height(), nullptr, program);
	}

	template <typename TEXTURE, class = typename std::enable_if<std::is_compound<TEXTURE>::value>::type>
	void image(TEXTURE t, const Program &program = get_program(TEXTURED_PROGRAM) ) const {
		draw_texture(t.id(), 0, 0, t.width(), t.height(), nullptr, program);
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void draw(const T &t, Program &program) const {
		draw_texture(t.id(), 0.0F, 0.0F, t.width(), t.height(), nullptr, program);
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type, typename V>
	void draw(const T &t, const V &pos) const {
		draw_texture(t.id(), pos[0], pos[1], t.width(), t.height());
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type, typename V>
	void draw(const T &t, const V &pos, float w, float h, const Program &program = get_program(TEXTURED_PROGRAM) ) const {
		draw_texture(t.id(), pos[0], pos[1], w, h, nullptr, program);
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void draw(const T &t, float x0, float y0, float w, float h, float *uvs, const Program &program = get_program(TEXTURED_PROGRAM) ) const {
		draw_texture(t.id(), x0, y0, w, h, uvs, program);
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void draw(const T &t, float x0 = 0, float y0 = 0, const Program &program = get_program(TEXTURED_PROGRAM) ) const {
		draw_texture(t.id(), x0, y0, t.width(), t.height(), nullptr, program);
	}

	void draw_textures(GLint texture, float *points, int count, float w, float h, float *uvs = nullptr, const Program &program = get_program(TEXTURED_PROGRAM) ) const;
	void draw_texture(GLint texture, float x0, float y0, float w, float h, float *uvs = nullptr, const Program &program = get_program(TEXTURED_PROGRAM) ) const;

	void clear(uint32_t color = 0xff000000);

	unsigned int width() const { return (unsigned int)_width; }
	unsigned int height() const { return (unsigned int)_height; }
	const Rectangle rec() const { return Rectangle(0,0,_width, _height); }
	GLuint buffer() const { return frameBuffer; }

	float scale() const { return globalScale; }
	float scale(float s) { globalScale = s; return s; }

	void text(const std::string &text, float x = 0, float y = 0, uint32_t color = 0xffffffff, float scale = 1.0) const;
	void text(const char c, float x = 0, float y = 0, uint32_t color = 0xffffffff, float scale = 1.0) const {
		static char s[2] = {0,0};
		s[0] = c;
		text(s, x, y, color, scale);
	}
	void text(const Font &font, const std::string &text, float x = 0, float y = 0, uint32_t color = 0xffffffff, float scale = 1.0) const;

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	void render(T renderable) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		renderable.render(_width, _height);
	}

	image::bitmap get_pixels() const {

		image::bitmap target(_width, _height);

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glReadPixels(0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, &target[0]);
		target.flip();
		return target;
	}

	const utils::mat4f& get_view_matrix() const { return toScreen; }

protected:

	void update_matrix() {
		toScreen = make_scale(2.0 / _width, -2.0 / _height);
		toScreen = make_translate(-1.0, 1.0, 0.5) * toScreen;
		LOGD("#### toScreen %d %d", _width, _height);
	}

	unsigned int frameBuffer;
	int _width;
	int _height;
	float globalScale;
	utils::mat4f toScreen;

	mutable std::shared_ptr<Font> font;

	static GLint circleBuf;
	static GLint recBuf;
	static GLint lineBuf;
	static GLint multiBuf[2];
};

}
#endif // GRAPPIX_BASIC_BUFFER_H
