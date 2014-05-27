#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

namespace grappix {

template <typename T = float> class Color {
public:
	Color(uint32_t c) {
		setColor(c);
	}

	void setColor(uint32_t color) {
		r = ((color>>16)&0xff)/255.0;
		g = ((color>>8)&0xff)/255.0;
		b = (color&0xff)/255.0;
		a = ((color>>24)&0xff)/255.0;
	}

	union {
		T data[4];
		struct {
			T r;
			T g;
			T b;
			T a;
		};
		struct {
			T red;
			T green;
			T blue;
			T alpha;
		};
	};
	operator uint32_t() {
		uint32_t c = ((int)(a*255)<<24) | ((int)(r*255)<<16) | ((int)(g*255)<<8) | (int)(b*255);
		return c;
	}

	float& operator[](int i) { return data[i]; }
	constexpr int size() const { return 4; }
};

Color<float> make_color(uint32_t color);

uint32_t blend(uint32_t col0, uint32_t col1, float alpha);

}

#endif // COLOR_H