#ifndef GRAPPIX_TEXTURE_H
#define GRAPPIX_TEXTURE_H

#include "basic_buffer.h"
#include "bitmap.h"

class texture : public basic_buffer {
public:
	texture() : texture_id(-1) {}

	template <typename T> texture(T size) : texture(size[0], size[1]) {}
	texture(int width, int height);

	texture(const bitmap &bm);

	unsigned int id() const { return texture_id; }
/*
	void draw(int x, int y, texture &buffer) {
		basic_buffer::draw_texture(buffer.id(), x, y, buffer.width(), buffer.height());
	};

	template <typename T> void draw(T pos, texture &buffer) {
		basic_buffer::draw_texture(buffer.id(), pos[0], pos[1], buffer.width(), buffer.height());
	};
*/
private:
	unsigned int texture_id;
};

#endif // GRAPPIX_TEXTURE_H
