#ifndef GRAPPIX_TEXTURE_H
#define GRAPPIX_TEXTURE_H

#include "render_target.h"
#include "bitmap.h"

class Texture : public RenderTarget {
public:
	Texture() : texture_id(-1) {}

	template <typename T> Texture(T size) : Texture(size[0], size[1]) {}
	Texture(int width, int height);

	Texture(const bitmap &bm);

	Texture clone() {
		Texture t  = Texture(width(), height());
		t.draw(*this, 0, 0);
		return t;
	}

	unsigned int id() const { return texture_id; }
/*
	void draw(int x, int y, Texture &buffer) {
		RenderTarget::draw_texture(buffer.id(), x, y, buffer.width(), buffer.height());
	};

	template <typename T> void draw(T pos, Texture &buffer) {
		RenderTarget::draw_Texture(buffer.id(), pos[0], pos[1], buffer.width(), buffer.height());
	};
*/
private:
	unsigned int texture_id;
};

#endif // GRAPPIX_TEXTURE_H
