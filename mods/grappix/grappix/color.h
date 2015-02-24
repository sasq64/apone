#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>
#include <coreutils/utils.h>

namespace grappix {

template <typename T = float> class Base_Color {
public:
	Base_Color() {}
	Base_Color(uint32_t c) {
		setColor(c);
	}

	Base_Color(T r, T g, T b, T a) : r(r), g(g), b(b), a(a) {}

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
	operator uint32_t() const {
		auto r0 = utils::clamp(r);
		auto g0 = utils::clamp(g);
		auto b0 = utils::clamp(b);
		auto a0 = utils::clamp(a);
		uint32_t c = ((int)(a0*255)<<24) | ((int)(r0*255)<<16) | ((int)(g0*255)<<8) | (int)(b0*255);
		return c;
	}

	Base_Color operator+(float x) {
		return Base_Color(r + x, g + x, b + x, a);
	}

	Base_Color operator+(const Base_Color &other) {
		return Base_Color(r + other.r, g + other.g, b + other.b, a + other.a);
	}

	Base_Color operator-(const Base_Color &other) {
		return Base_Color(r - other.r, g - other.g, b - other.b, a - other.a);
	}

	Base_Color operator/(float x) {
		return Base_Color(r / x, g / x, b / x, a);
	}

	Base_Color operator*(float x) {
		return Base_Color(r * x, g * x, b * x, a);
	}

	float& operator[](int i) { return data[i]; }
	//constexpr int size() const { return 4; }

	T *begin() { return std::begin(data); }
	T *end() { return std::end(data); }

	static const Base_Color<T> WHITE;
	static const Base_Color<T> RED;
	static const Base_Color<T> GREEN;
	static const Base_Color<T> BLUE;
	static const Base_Color<T> YELLOW;
	static const Base_Color<T> BLACK;

};

typedef Base_Color<float> Color;

//Color make_color(uint32_t color);

uint32_t blend(uint32_t col0, uint32_t col1, float alpha);

}

#endif // COLOR_H