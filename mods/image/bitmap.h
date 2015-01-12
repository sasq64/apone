#ifndef IMAGE_BITMAP_H
#define IMAGE_BITMAP_H

#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <memory>

using namespace std;

namespace image {

template <typename T = uint32_t> class basic_bitmap {

	class const_split_iterator  {
	public:
		const_split_iterator(const basic_bitmap &bm, int w, int h, int xpos = 0, int ypos = 0) : bm(bm), width(w), height(h), xpos(xpos), ypos(ypos) {}
		const_split_iterator(const const_split_iterator& rhs) : bm(rhs.bm), width(rhs.width), height(rhs.height), xpos(rhs.xpos), ypos(rhs.ypos) {}

		bool operator!= (const const_split_iterator& other) const {
			return xpos != other.xpos || ypos != other.ypos;
		}

		basic_bitmap operator* () const {
			return bm.cut(xpos, ypos, width, height);
		}

		const const_split_iterator& operator++ () {
			xpos += width;
			if(xpos > bm.width()-width) {
				xpos = 0;
				ypos += height;
				if(ypos > bm.height()-height) {
					xpos = ypos = -1;
				}
			}
			return *this;
		}
	private:
		const basic_bitmap &bm;
		int width;
		int height;
		int xpos;
		int ypos;
	};

	class splitter {
	public:
		splitter(const basic_bitmap &bm, int w, int h) : bm(bm), width(w), height(h) {}
		const_split_iterator begin() {
			return const_split_iterator(bm, width, height);
		}

		const_split_iterator end() {
			return const_split_iterator(bm, width, height, -1, -1);
		}
	private:
		const basic_bitmap &bm;
		int width;
		int height;
	};

public:

	splitter split(int w, int h) const {
		return splitter(*this, w, h);
	}


	basic_bitmap() : w(0), h(0) {}

	basic_bitmap(int width, int height) : w(width), h(height) {
		//pixels.resize(width * height);
		pixels = make_shared<vector<T>>(width*height);
	}

	basic_bitmap(int width, int height, const T& color) : w(width), h(height) {
		//pixels.resize(width * height);
		pixels = make_shared<vector<T>>(width*height);
		std::fill(pixels->begin(), pixels->end(), color);
	}

	basic_bitmap(int width, int height, const T *px) : w(width), h(height) {
		//pixels.resize(width * height);
		pixels = make_shared<vector<T>>(width*height);
		memcpy(&(*pixels)[0], px, sizeof(T) * width * height);
	}

	void put(int x, int y, const basic_bitmap &bm) {
		for(int yy = 0; yy < bm.h; yy++)
			for(int xx = 0; xx < bm.w; xx++) {
				(*pixels)[xx + x + (yy + y)*w] = bm[xx+yy*bm.w];
			}
		dirty = true;
	}

	basic_bitmap clone() {
		return basic_bitmap(w, h, &(*pixels)[0]);
	}

	T& operator[](const int &i) {
		dirty = true;
		return (*pixels)[i];
	}

	T operator[](const int &i) const { return (*pixels)[i]; }

	typename std::vector<T>::iterator begin() {
		return pixels->begin();
	};

	typename std::vector<T>::iterator end() {
		return pixels->end();
	};

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

	const T* data() const { return &(*pixels)[0]; }

	const T* flipped() const {

		if(dirty) {
			 flipPixels = std::make_shared<vector<T>>(w*h);
			int l = sizeof(T) * w;
			flipPixels->resize(w*h);
			for(int y=0; y<h; y++)
				std::memcpy(&(*flipPixels)[y*w], &(*pixels)[(h-y-1)*w], l);
			dirty = false;
		}
		return &(*flipPixels)[0];
	}

	void flip() {
		if(!flipPixels)
			flipPixels = std::make_shared<vector<T>>(w*h);
		flipPixels->resize(w*h);
		*flipPixels = *pixels;
		int l = sizeof(T) * w;
		for(int y=0; y<h; y++)
			std::memcpy(&(*pixels)[(h-y-1)*w], &(*flipPixels)[y*w], l);
	}


	unsigned int width() const { return w; }
	unsigned int height() const { return h; }
	unsigned int size() const { return w*h; }
private:
	std::shared_ptr<std::vector<T>> pixels;
	mutable std::shared_ptr<std::vector<T>> flipPixels;
	mutable bool dirty = true;
	unsigned int w;
	unsigned int h;
};

typedef basic_bitmap<uint32_t> bitmap;
typedef basic_bitmap<uint8_t> bitmap8;

}

#endif // IMAGE_BITMAP_H