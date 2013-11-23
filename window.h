#ifndef GRAPPIX_WINDOW_H
#define GRAPPIX_WINDOW_H

#include "texture.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <chrono>
#include <unordered_map>
#include <deque>
#include <functional>

class window : public basic_buffer {
public:

	window();

	//void draw(int x, int y, texture &buffer) {
	//	basic_buffer::draw_texture(buffer.id(), x, y, buffer.width(), buffer.height());
	//};

	//template <typename T> void draw(T pos, texture &buffer) {
	//	basic_buffer::draw_texture(buffer.id(), pos[0], pos[1], buffer.width(), buffer.height());
	//};

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
		CLICK,
		RIGHT_CLICK,
		NO_KEY = 256
	};

	struct click {
		int x;
		int y;
		int button;
	};

	click NO_CLICK;

	click get_click();

	bool key_pressed(key k);
	key get_key();

	bool is_open() { return winOpen; }
	std::pair<float, float> size() { return std::make_pair(_width, _height); }

	void renderLoop(std::function<void()> f);

	constexpr static const double FPS = 1.0/60.0;

	void benchmark(); 
	void resize(int w, int h) {
		_width = w;
		_height = h;
	}

	static std::deque<int> key_buffer;
	static std::deque<click> click_buffer;

private:
	int lastTime;
	bool winOpen;
	unsigned int bmCounter;
	std::chrono::high_resolution_clock::time_point benchStart;

	std::chrono::high_resolution_clock::time_point startTime;

	static std::unordered_map<int, int> translate;
};

extern window screen;

#endif // GRAPPIX_WINDOW_H