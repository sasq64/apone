#ifndef GRAPPIX_TEXTURE_H
#define GRAPPIX_TEXTURE_H

#include "render_target.h"
#include "bitmap.h"

#include <memory>

namespace grappix {

class Texture : public RenderTarget {
public:

	enum Format {
		ALPHA8 = 1,
		RGB16,
		RGB24,
		RGBA32
	};

	Texture() {}

	template <typename T> Texture(T size) : Texture(size[0], size[1]) {}
	Texture(unsigned int width, unsigned int height, Format fmt = RGBA32);
	Texture(const bitmap &bm);
	Texture(uint8_t *data, unsigned int w, unsigned int h);
	unsigned int id() const { return tref->id; }
private:

	struct texref {
		texref() {
			LOGD("GEN");
			glGenTextures(1, &id);
		}
		texref(GLuint id) : id(id) {}
		~texref() {
			LOGD("DEL");
			glDeleteTextures(1, &id);
		}
		GLuint id;
	};

	std::shared_ptr<texref> tref;
};

}

#endif // GRAPPIX_TEXTURE_H
