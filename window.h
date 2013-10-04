#ifndef GRAPPIX_WINDOW_H
#define GRAPPIX_WINDOW_H

#include "renderbuffer.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <chrono>

class window : public basic_buffer {
public:

	window() : basic_buffer(), winOpen(false), bmCounter(0) {
		open();
	}
	/*window(int width, int height) : basic_buffer(0, width, height), winOpen(false) {
		open();
	}*/

	void draw(renderbuffer &buffer, int x, int y) {
		basic_buffer::draw(buffer.texture(), x, y, buffer.getWidth(), buffer.getHeight());
	};

	template <typename T> void draw(T pos, renderbuffer &buffer) {
		basic_buffer::draw(buffer.texture(), pos[0], pos[1], buffer.getWidth(), buffer.getHeight());
	};

	void open();
	void flip();

	bool is_open() { return winOpen; }
	std::pair<float, float> size() { return std::make_pair(width, height); }

	constexpr static const double FPS = 1.0/60.0;

	void benchmark(); 

private:
	int lastTime;
	bool winOpen;
	uint bmCounter;
	std::chrono::high_resolution_clock::time_point benchStart;
};

extern window screen;

#endif // GRAPPIX_WINDOW_H