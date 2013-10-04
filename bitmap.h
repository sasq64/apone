#ifndef GRAPPIX_BITMAP_H
#define GRAPPIX_BITMAP_H

#include <cstdint>
#include <vector>

template <typename T = uint32_t> class basic_bitmap {
public:
	basic_bitmap(int width, int height) : w(width), h(height) {
		data.resize(width * height);
	}
	T& operator[](const int &i) { return data[i]; }
	int width() { return w; }
	int height() { return h; }
private:
	std::vector<T> data;
	int w;
	int h;
};

typedef basic_bitmap<uint32_t> bitmap;

#endif // GRAPPIX_BITMAP_H