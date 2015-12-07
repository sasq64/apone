#ifndef FONT_H
#define FONT_H

//#include "render_target.h"
#include "shader.h"
#include <coreutils/vec.h>
#include <coreutils/log.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

//struct texture_atlas_t;
//struct texture_font_t;

namespace grappix {

class RenderTarget;

template <typename T, typename V> class VBLCache {

struct Entry {
	Entry() {}
	Entry(const V &v) : value(v), age(1) {}
	V value;
	int age;
};

public:
	void put(const T &id, const V &value) {
		map[id] = Entry(value);
	}
	V get(const T &id) {
		auto &r = map[id];
		r.age++;
		return r.value;
	}

	void cleanup() {
		auto it = map.begin();
		while(it != map.end()) {
			if(it->second.age == 0) {
				it->second.value.destroy();
				map.erase(it++);
				//LOGD("Erasing");
			} else {
				it->second.age--;
				it++;
			}
		}
	}

private:
	std::unordered_map<T, Entry> map;
};


struct TextBuf {
	TextBuf() : vbuf(2) {}
	int size;
	std::vector<GLuint> vbuf;
	std::wstring text;
	float rec[4];
	void destroy();
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

	TextBuf make_text(const std::wstring &text) const;
	void render_text(const RenderTarget &target, const TextBuf &text, float x = 0, float y = 0, uint32_t color = 0xffffffff, float scale = 1.0) const;
	void render_text(const RenderTarget &target, const std::string &text, float x = 0, float y = 0, uint32_t col = 0xffffffff, float scale = 1.0) const;

	int get_width(const std::string &text, float scale) const;
	utils::vec2i get_size(const std::string &text, float scale) const;

	void set_program(const Program &p) {
		program = p;
	}

	void update_cache() {
		cache.cleanup();
	}

private:

	int size;

	Program program;

	class FontRef {
	public:
		FontRef(int w, int h, const std::string &ttfName, int fsize, int flags);
		~FontRef();
		int w, h;
		int flags;
		std::string ttfName;
		void *atlas;
		void *font;
	};

	static std::vector<std::weak_ptr<FontRef>> fontRefs;

	TextBuf make_text2(const std::wstring &text) const;

	std::shared_ptr<FontRef> ref;

	mutable VBLCache<std::wstring, TextBuf> cache;
};

}

#endif // FONT_H
