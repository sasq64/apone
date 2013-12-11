#ifndef FONT_H
#define FONT_H

//#include "render_target.h"
#include "texture-font.h"
#include "texture-atlas.h"
class RenderTarget;

#include <vector>
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


struct TextBuf {
	TextBuf() : vbuf(2) {}
	std::vector<GLuint> vbuf;
	std::string text;
};

class Font {
public:

	enum {
		DISTANCE_MAP = 1
	};
	Font();
	Font(const std::string &ttfName, int size, int flags = DISTANCE_MAP);
	TextBuf make_text(const std::string &text);
	void render_text(RenderTarget &target, const TextBuf &text, int x = 0, int y = 0, uint32_t color = 0xffffffff, float scale = 1.0);
	void render_text(RenderTarget &target, const std::string &text, int x = 0, int y = 0, uint32_t col = 0xffffffff, float scale = 1.0);

private:
	TextBuf make_text2(const std::string &text);

	texture_font_t *font;
	texture_atlas_t *atlas;
	VBLCache<std::string, TextBuf> cache;
};

#endif // FONT_H