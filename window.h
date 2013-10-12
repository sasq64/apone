#ifndef GRAPPIX_WINDOW_H
#define GRAPPIX_WINDOW_H

#include "renderbuffer.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <chrono>
#include <unordered_map>
#include <deque>

class window : public basic_buffer {
public:

	window() : basic_buffer(), winOpen(false), bmCounter(0) {
	}
	/*window(int width, int height) : basic_buffer(0, width, height), winOpen(false) {
		open();
	}*/

	void draw(int x, int y, renderbuffer &buffer) {
		basic_buffer::draw_texture(buffer.texture(), x, y, buffer.width(), buffer.height());
	};

	template <typename T> void draw(T pos, renderbuffer &buffer) {
		basic_buffer::draw_texture(buffer.texture(), pos[0], pos[1], buffer.width(), buffer.height());
	};

	void open(bool fs);
	void open(int w = 0, int h = 0, bool fs = true);
	void flip();
	void vsync();

	enum key {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		ENTER,
		SPACE,
		NO_KEY = 256
	};

	bool key_pressed(key k);
	key get_key();

	bool is_open() { return winOpen; }
	std::pair<float, float> size() { return std::make_pair(_width, _height); }

	constexpr static const double FPS = 1.0/60.0;

	void benchmark(); 

	static std::deque<int> key_buffer;

private:
	int lastTime;
	bool winOpen;
	uint bmCounter;
	std::chrono::high_resolution_clock::time_point benchStart;

	static std::unordered_map<int, int> translate;
};

extern window screen;

#endif // GRAPPIX_WINDOW_H