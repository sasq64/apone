#include "color.h"

namespace grappix {

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
