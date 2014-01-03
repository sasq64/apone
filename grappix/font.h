#ifndef FONT_H
#define FONT_H

//#include "render_target.h"
#include "texture-font.h"
#include "texture-atlas.h"
#include <coreutils/log.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace grappix {

class RenderTarget;

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


struct TextBuf {
	TextBuf() : vbuf(2) {}
	int size;
	std::vector<GLuint> vbuf;
	std::string text;
};

class Font {
public:

	enum {
		DISTANCE_MAP = 1,
		UPPER_CASE = 2,
		SIZE_128 = 128,
		SIZE_256 = 256,
		SIZE_512 = 512
	};
	Font() {};
	Font(bool s);
	Font(const std::string &ttfName, int size, int flags = DISTANCE_MAP);

	TextBuf make_text(const std::string &text) const;
	void render_text(const RenderTarget &target, const TextBuf &text, int x = 0, int y = 0, uint32_t color = 0xffffffff, float scale = 1.0) const;
	void render_text(const RenderTarget &target, const std::string &text, int x = 0, int y = 0, uint32_t col = 0xffffffff, float scale = 1.0) const;

private:

	class FontRef {
	public:
		FontRef(int w, int h, const std::string &ttfName, int size) : atlas(nullptr), font(nullptr) {
			LOGD("FONTREF CONSTRUCT");
			if(w > 0 && h > 0)
				atlas = texture_atlas_new(w, h, 1 );
			if(atlas && size > 0) {
				font = texture_font_new(atlas, ttfName.c_str(), size);
				LOGD("FONTREF DONE");
			}
		}
		~FontRef() {
			LOGD("FONTREF DESTROY");
			texture_font_delete(font);
			texture_atlas_delete(atlas);
			font = nullptr;
			atlas = nullptr;
		}
		texture_atlas_t *atlas;
		texture_font_t *font;
	};

	TextBuf make_text2(const std::string &text) const;

	std::shared_ptr<FontRef> ref;

	mutable VBLCache<std::string, TextBuf> cache;
};

}

#endif // FONT_H