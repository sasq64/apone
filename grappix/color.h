#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

template <typename T = float> class Color {
public:
	T red;
	T green;
	T blue;
	T alpha;
};

Color<float> make_color(uint32_t color);

uint32_t blend(uint32_t col0, uint32_t col1, float alpha);

#endif // COLOR_H