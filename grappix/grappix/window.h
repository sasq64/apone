#ifndef GRAPPIX_WINDOW_H
#define GRAPPIX_WINDOW_H

#include "texture.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <chrono>
#include <unordered_map>
#include <set>
#include <deque>
#include <functional>
#include <tuple>
#include <mutex>
#include <thread>

namespace grappix {

class display_exception : public std::exception {
public:
	display_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};


class Window : public RenderTarget {
public:

	Window();

	void open(bool fs = true);
	void open(int w, int h, bool fs = false);
	void flip();
	void vsync();

	enum key {
		SPACE = 0x20,
		UP = 0x100,
		DOWN,
		LEFT,
		RIGHT,
		ENTER,
		ESCAPE,
		BACKSPACE,
		TAB,
		PAGEUP,
		PAGEDOWN,
		DELETE,
		HOME,
		END,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		SHIFT_LEFT = 0x200,
		SHIFT_RIGHT,
		ALT_LEFT,
		ALT_RIGHT,
		CTRL_LEFT,
		CTRL_RIGHT,
		WINDOWS,
		CLICK = 0x300,
		RIGHT_CLICK,
		NO_KEY = 0xffff
	};

	struct click {
		click(int xx = 0, int yy = 0, int b = 0) : x(xx), y(yy), button(b) {}
		bool operator==(const click &c) { return c.x == x && c.y == y && c.button == c.button; }
		bool operator!=(const click &c) { return !(*this == c); }
		int x;
		int y;
		int button;
	};

	static click NO_CLICK;

	click get_click(bool peek = false);
	bool mouse_pressed();
	std::tuple<int, int> mouse_position();

	bool key_pressed(key k);
	bool key_pressed(char k);
	key get_key(bool peek = false);

	bool is_open() { return winOpen; }
	std::pair<float, float> size() { return std::make_pair(_width, _height); }

	//void render_loop(std::function<void()> f, int fps = 60);
	void render_loop(std::function<void(uint32_t)> f, int fps = 60);

	void on_focus(std::function<void()> f) { focus_func = f; }
	void on_focus_lost(std::function<void()> f) { focus_lost_func = f; }

	constexpr static const double FPS = 1.0/60.0;

	void resize(int w, int h) {
		_width = w;
		_height = h;
	}

	static std::deque<int> key_buffer;
	static std::deque<click> click_buffer;


	void setup(int w, int h) {

		if(winOpen)
			return;

		_width = w;
		_height = h;

		LOGD("Window %dx%d", _width, _height);

		update_matrix();

		glLineWidth(2.0);
		glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		//glBlendColor(1.0,1.0,1.0,1.0);
		lastTime = -1;
		winOpen = true;

		startTime = chrono::high_resolution_clock::now();
		frameBuffer = 0;
	}

	int call_repeatedly(std::function<void(void)> f, int msec) {
		callbacks.push_back(Callback(f, msec));
		return callbacks.size()-1;
	}

	void update_callbacks() {

		while(safeFuncs.size() > 0) {
			safeMutex.lock();
			auto &f = safeFuncs.front();
			f();
			safeFuncs.pop_front();
			safeMutex.unlock();
		}

		auto ms = utils::getms();
		for(auto &cb : callbacks) {
			if(cb.msec == 0 || ms >= cb.next_time) {
				cb.cb();
				cb.next_time += cb.msec;
			}
		}

		for(auto i : to_remove) {
			callbacks.erase(callbacks.begin() + i);
		}
		to_remove.clear();
	}

	void remove_repeating(int index) {
		to_remove.insert(index);
	}

	void benchmark() {
		benchStart = chrono::high_resolution_clock::now();
		bmCounter = 100;
	}


	void lock() {
		lockIt = true;
	}

	void unlock() {
		lockIt = false;
	}

	bool locked() { return lockIt; }

	void run_safely(std::function<void()> f) {
		safeMutex.lock();
		safeFuncs.push_back(f);
		safeMutex.unlock();
		while(safeFuncs.size() != 0) {
			safeMutex.lock();
			safeMutex.unlock();
		}
	}


	std::function<void()> focus_func;
	std::function<void()> focus_lost_func;

private:


	//void update_callbacks();

	struct Callback {
		Callback(std::function<void(void)> cb, int msec) : cb(cb), msec(msec) {
			next_time = utils::getms() + msec;
		}
		std::function<void(void)> cb;
		unsigned long msec;
		unsigned long next_time;
	};



	std::vector<Callback> callbacks;
	std::set<int, std::greater<int>> to_remove;

	int64_t lastTime;
#ifdef FPS_COUNTER
	float fps;
#endif
	bool winOpen;
	unsigned int bmCounter;
	std::chrono::high_resolution_clock::time_point benchStart;

	std::chrono::high_resolution_clock::time_point startTime;

	std::atomic<bool> lockIt;

	std::mutex safeMutex;
	std::deque<std::function<void()>> safeFuncs;

	static std::unordered_map<int, int> translate;

	std::thread keyboardThread;
};


constexpr Window::key as_key(const char c) {
	return static_cast<Window::key>(c); 
}

extern Window screen;
}

#endif // GRAPPIX_WINDOW_H