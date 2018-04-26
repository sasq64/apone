#ifndef GRAPPIX_TEXTURE_H
#define GRAPPIX_TEXTURE_H

#include "render_target.h"
#include <image/bitmap.h>
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

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
		Texture(T size) : Texture(size[0], size[1]) {}

	Texture(int width, int height, Format fmt = RGBA32);
	Texture(const image::bitmap &bm);
	Texture(const uint8_t *data, unsigned int w, unsigned int h);
	unsigned int id() const { return tref->id; }
	void putPixels(const void *data);
	template<typename T> void putPixels(const std::unique_ptr<T>& data)
	{
		putPixels(data.get());
	}
private:

	struct texref {
		texref();
		texref(GLuint id) : id(id) {}
		~texref();
		GLuint id;
	};

	std::shared_ptr<texref> tref;
};

}

#endif // GRAPPIX_TEXTURE_H
