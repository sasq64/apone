#include "color.h"

namespace grappix {

template <typename T> const Base_Color<T> Base_Color<T>::WHITE = Base_Color<T>(0xffffffff);
template <typename T> const Base_Color<T> Base_Color<T>::RED = Base_Color<T>(0xffff0000);
template <typename T> const Base_Color<T> Base_Color<T>::GREEN = Base_Color<T>(0xff00ff00);
template <typename T> const Base_Color<T> Base_Color<T>::BLUE = Base_Color<T>(0xff0000ff);
template <typename T> const Base_Color<T> Base_Color<T>::YELLOW = Base_Color<T>(0xffffff00);
template <typename T> const Base_Color<T> Base_Color<T>::BLACK = Base_Color<T>(0xff000000);

template <> const Base_Color<float> Base_Color<float>::WHITE = Base_Color<float>(0xffffffff);
template <> const Base_Color<float> Base_Color<float>::RED = Base_Color<float>(0xffff0000);
template <> const Base_Color<float> Base_Color<float>::GREEN = Base_Color<float>(0xff00ff00);
template <> const Base_Color<float> Base_Color<float>::BLUE = Base_Color<float>(0xff0000ff);
template <> const Base_Color<float> Base_Color<float>::YELLOW = Base_Color<float>(0xffffff00);
template <> const Base_Color<float> Base_Color<float>::BLACK = Base_Color<float>(0xff000000);
//template <> const Base_Color<float> Base_Color<float>::WHITE = Base_Color<float>(0xffffffff);

Color make_color(uint32_t color) {
	return Color(color);
}

uint32_t blend(uint32_t col0, uint32_t col1, float alpha) {
	auto a0 = (col0>>24)&0xff;
	auto r0 = (col0>>16)&0xff;
	auto g0 = (col0>>8)&0xff;
	auto b0 = col0&0xff;

	auto a1 = (col1>>24)&0xff;
	auto r1 = (col1>>16)&0xff;
	auto g1 = (col1>>8)&0xff;
	auto b1 = col1&0xff;

	a0 = a0*alpha+a1*(1.0-alpha);
	r0 = r0*alpha+r1*(1.0-alpha);
	g0 = g0*alpha+g1*(1.0-alpha);
	b0 = b0*alpha+b1*(1.0-alpha);
	return (a0<<24) | (r0<<16) | (g0<<8) | b0;
}

}