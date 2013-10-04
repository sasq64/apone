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