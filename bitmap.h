#ifndef GRAPPIX_BITMAP_H
#define GRAPPIX_BITMAP_H

#include <cstdint>
#include <vector>

template <typename T = uint32_t> class basic_bitmap {
public:
	basic_bitmap(int width, int height) : w(width), h(height) {
		pixels.resize(width * height);
	}
	T& operator[](const int &i) { return pixels[i]; }
	T operator[](const int &i) const { return pixels[i]; }

	basic_bitmap operator+(T &pixel) {
		basic_bitmap rb = *this;
		for(T &p : rb.pixels)
			p += pixel;
		return rb;
	}

	std::vector<basic_bitmap> split(int w, int h) {
		std::vector<basic_bitmap> rv;
		return rv;
	}

	const T* data() const { return &pixels[0]; }

	int width() const { return w; }
	int height() const { return h; }
	int size() const { return w*h; }
private:
	std::vector<T> pixels;
	int w;
	int h;
};

typedef basic_bitmap<uint32_t> bitmap;

#endif // GRAPPIX_BITMAP_H