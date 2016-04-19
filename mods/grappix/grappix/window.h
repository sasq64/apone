#ifndef GRAPPIX_WINDOW_H
#define GRAPPIX_WINDOW_H

#include "texture.h"
#include "resources.h"
#include <tween/tween.h>
#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <coreutils/events.h>
#include <chrono>
#include <unordered_map>
#include <set>
#include <deque>
#include <functional>
#include <tuple>
//#include <coreutils/thread.h>
#include <mutex>
#include <thread>

#ifdef _WIN32
#undef DELETE
#endif

namespace grappix {

class display_exception : public std::exception {
public:
	display_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

struct KeyEvent {
	KeyEvent(int c) : code(c) {}
	int code;
};

class Window : public RenderTarget {
public:

	Window();
	~Window();

	void open(bool fs = true);
	void open(int w, int h, bool fs = false);
	void flip();
	void vsync();
	void close();

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
		WINDOW_LEFT,
		WINDOW_RIGHT,
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

	struct Scroll {
		Scroll(double x, double y) : x(x), y(y) {}
		bool operator==(const Scroll &c) { return c.x == x && c.y == y; }
		bool operator!=(const Scroll &c) { return !(*this == c); }
		double x;
		double y;
	};

	static click NO_CLICK;
	static Scroll NO_SCROLL;

	click get_click(bool peek = false);
	Scroll get_scroll(bool peek = false);
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

	void resize(int w, int h);

	static std::deque<click> click_buffer;
	static std::deque<Scroll> scroll_buffer;


	void setup(int w, int h);
	int call_repeatedly(std::function<void(void)> f, int msec);
	void update_callbacks();
	void remove_repeating(int index);
	void benchmark();
	
	int getPPI() const { return ppi; }
	
	void setTitle(const std::string &t) { title = t; }

	void lock() {
		lockIt = true;
	}

	void unlock() {
		lockIt = false;
	}

	bool locked() { return lockIt; }

	void run_safely(std::function<void()> f);

	std::function<void()> focus_func;
	std::function<void()> focus_lost_func;

	static std::unordered_map<int, int> translate;

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

	std::thread keyboardThread;
	
	int ppi = -1;
	std::string title;
};


constexpr Window::key as_key(const char c) {
	return static_cast<Window::key>(c);
}

extern Window &screen;
extern std::shared_ptr<Window> screenptr;
}

#endif // GRAPPIX_WINDOW_H
