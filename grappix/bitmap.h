#ifndef GRAPPIX_BITMAP_H
#define GRAPPIX_BITMAP_H

//#include <coreutils/log.h>

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <memory>

using namespace std;

namespace grappix {

template <typename T = uint32_t> class basic_bitmap {
public:
	basic_bitmap() : w(0), h(0) {}

	//basic_bitmap(basic_bitmap &&b) : pixels(std::move(b.pixels)), w(b.w), h(b.h) {}

	//basic_bitmap& operator=(const basic_bitmap &b) {
	//	return *this;
	//}

	//basic_bitmap& operator=(const basic_bitmap &&b) {
	//	pixels = std::move(b.pixels);
	//	w = b.w;
	//	h = b.h;
	//	return *this;
	//}

	basic_bitmap(int width, int height) : w(width), h(height) {
		//pixels.resize(width * height);
		pixels = make_shared<vector<T>>(width*height);
	}

	basic_bitmap(int width, int height, void *px) : w(width), h(height) {
		//pixels.resize(width * height);
		pixels = make_shared<vector<T>>(width*height);
		memcpy(&(*pixels)[0], px, sizeof(T) * width * height);
	}

	basic_bitmap clone() {
		return basic_bitmap(w, h, &(*pixels)[0]);
	}

	T& operator[](const int &i) { 
		return (*pixels)[i];
	}

	T operator[](const int &i) const { return pixels[i]; }

	basic_bitmap operator+(T &pixel) {
		basic_bitmap rb = *this;
		for(T &p : rb.pixels)
			p += pixel;
		return rb;
	}

	void clear(uint32_t color = 0) {
		T * p = &(*pixels)[0];
		if(!color)
			memset(p, 0, w*h*sizeof(T));
		else {
			for(int i=0; i<w*h; i++)
				p[i] = color;
		}
	}

	basic_bitmap cut(int x, int y, int ww, int hh) const {
		basic_bitmap dest(ww, hh);
		T *p = &(*pixels)[0];
		for(int yy=0; yy<hh; yy++)
			for(int xx=0; xx<ww; xx++) {
				dest[xx+yy*ww] = p[xx+x+(yy+y)*this->w];
			}
		return dest;
	}

	std::vector<basic_bitmap> split(int w, int h) {
		std::vector<basic_bitmap> rv;
		return rv;
	}

	const T* data() const { return &(*pixels)[0]; }

	const T* flipped() const {

		int l = sizeof(T) * w;
		flipPixels.resize(w*h);
		for(int y=0; y<h; y++)
			std::memcpy(&flipPixels[y*w], &(*pixels)[(h-y-1)*w], l);
		return &flipPixels[0];
	}


	int width() const { return w; }
	int height() const { return h; }
	int size() const { return w*h; }
private:
	std::shared_ptr<std::vector<T>> pixels;
	mutable std::vector<T> flipPixels;
	bool dirty;
	int w;
	int h;
};

typedef basic_bitmap<uint32_t> bitmap;

}

#endif // GRAPPIX_BITMAP_H