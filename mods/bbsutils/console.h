#ifndef BBS_CONSOLE_H
#define BBS_CONSOLE_H


#include "terminal.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>
#include <coreutils/vec.h>

#include <stdint.h>
//#include <termios.h>
#include <time.h>
#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <initializer_list>

namespace bbs {

class Console {
public:	

	enum Color {
		WHITE,
		RED,
		GREEN,
		BLUE,
		ORANGE,
		BLACK,
		BROWN,
		PINK,
		DARK_GREY,
		GREY,
		LIGHT_GREEN,
		LIGHT_BLUE,
		LIGHT_GREY,
		PURPLE,
		YELLOW,
		CYAN,
		CURRENT_COLOR = -2, // Use the currently set fg or bg color
		NO_COLOR = -1
	};

	enum {
		KEY_ESCAPE = 0x1b,
		KEY_BACKSPACE = 0x10000,
		KEY_LEFT,
		KEY_UP,
		KEY_RIGHT,
		KEY_DOWN,
		KEY_PAGEUP,
		KEY_PAGEDOWN,
		KEY_HOME,
		KEY_END,
		KEY_ENTER,
		KEY_TAB,
		KEY_DELETE,

		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,

		KEY_UNKNOWN = 0x1fffe,
		KEY_TIMEOUT = 0x1ffff

	};

	enum {
		CODE_CRLF = 0x2028
	};

	typedef uint16_t Char;

	struct Tile {
		Tile(Char c = ' ', int fg = -1, int bg = -1) : fg(fg), bg(bg), c(c) {}
		Tile& operator=(std::initializer_list<int> il) {
			auto it = il.begin();
			c = *it;
			++it;
			fg = *it;
			++it;
			bg = *it;
			return *this;
		}
		bool operator==(const Tile &o) const {
   			return (fg == o.fg && bg == o.bg && c == o.c);
  		}
  		bool operator!=(const Tile &o) const {
  			return !(*this == o);
  		}

		int fg;
		int bg;
		Char c;
	};

	Console(Terminal &terminal) : terminal(terminal), fgColor(WHITE), bgColor(BLACK), width(40), height(25), curX(0), curY(0), raw_mode(false) {
		grid.resize(width*height);
		oldGrid.resize(width*height);
		clipX0 = clipY0 = 0;
		clipX1 = width;
		clipY1 = height;
		terminal.open();
	}

	~Console() {
		showCursor(true);
		//flush(false);
		terminal.close();
	}

	virtual int getKey(int timeout = -1);
	virtual void clear();
	virtual void put(int x, int y, const std::string &text, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void put(int x, int y, const std::wstring &text, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void put(int x, int y, const std::vector<uint32_t> &text, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void put(int x, int y, Char c, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void put(const std::string &text, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void write(const std::string &text);
	virtual void write(const std::wstring &text);
	virtual void write(const std::vector<uint32_t> &text);

	//template <class... A> void write(const std::string &fmt, A... args) {
	//	std::string s = utils::format(fmt, args...);
	//	write(s);
	//}

	virtual void setColor(int fg, int bg = BLACK);
	virtual void resize(int w, int h);
	virtual void flush(bool restoreCursor = true);
	virtual void putChar(Char c);
	virtual void moveCursor(int x, int y);
	virtual void fill(int bg = CURRENT_COLOR, int x = 0, int y = 0, int width = 0, int height = 0);

	virtual void fillLine(int y, int bg = CURRENT_COLOR) {
		fill(bg, 0, y, 0, 1);
	}

	virtual void refresh();

	virtual void scrollScreen(int dy);
	//virtual void scrollLine(int dx);

	virtual std::string getLine(int maxlen = 0);
	virtual std::string getLine(const std::string &prompt, int maxlen = 0) {
		write(prompt);
		return getLine(maxlen);
	}
	virtual std::string getPassword(int maxlen = 0);
	virtual std::string getPassword(const std::string &prompt, int maxlen = 0) {
		write(prompt);
		return getPassword(maxlen);
	}

	void set_raw(bool m) { raw_mode = m; }
/*
	virtual std::future<std::string> getLineAsync() {
		getLineStarted = false;
		auto rc = std::async(std::launch::async, &Console::getLine, this);
		while(!getLineStarted) {
    		std::this_thread::sleep_for(std::chrono::milliseconds(100));
    	}
    	return rc;
	};
*/

	virtual const std::vector<Tile> &getTiles() const {
		return grid;
	}
	virtual void setTiles(const std::vector<Tile> &tiles) {
		grid = tiles;
	}

	int getWidth() const { return width; }
	int getHeight() const { return height; }
	utils::vec<int, 2> getSize() const { return utils::vec<int, 2>(width, height); }
	int getCursorX() const { return curX; }
	int getCursorY() const { return curY; }

	utils::vec<int, 2> getCursor() const { return utils::vec<int, 2>(curX, curY); }
	void moveCursor(const utils::vec<int, 2> &pos) { moveCursor(pos.x, pos.y); }
	void crlf() { moveCursor(0, curY++); }
	
	virtual void showCursor(bool show) {
		impl_showcursor(show);
	}
	

	void clipArea(int x = 0, int y = 0, int w = -1, int h = -1) {
		if(w <= 0)  w = width-w;
		if(h <= 0)  h = height-h;
		clipX0 = x;
		clipY0 = y;
		clipX1 = x + w;
		clipY1 = y + h;
	}

	int getFg() const { return fgColor; }
	int getBg() const { return bgColor; }

	//void rawPut(Char c) {
	//	outBuffer.push_back(c & 0xff);
	//}

	virtual const std::string name() const = 0;

	static Console *createLocalConsole();

protected:

	void shiftTiles(std::vector<Tile> &tiles, int dx, int dy);
	void clearTiles(std::vector<Tile> &tiles, int x0, int y0, int w, int h);

	int get_utf8();

	// Functions that needs to be implemented by real console implementations

	virtual void impl_color(int fg, int bg) = 0;
	virtual void impl_gotoxy(int x, int y) = 0;
	virtual bool impl_scroll_screen(int dy) { return false; }
	virtual bool impl_scroll_line(int dx) { return false; }
	virtual int impl_handlekey() = 0;
	virtual void impl_clear() = 0;
	virtual void impl_showcursor(bool show) {}
	
	virtual void impl_translate(Char &c) {
		if(c == '\t') c = ' ';
	}


	Terminal &terminal;

	// Outgoing raw data to the terminal
	std::vector<uint8_t> outBuffer;

	// Incoming raw data from the terminal
	std::queue<uint8_t> inBuffer;

	// The contents of the screen after next flush.
	std::vector<Tile> grid;
	// The contents on the screen now. The difference is used to
	// send characters to update the console.
	std::vector<Tile> oldGrid;


	int fgColor;
	int bgColor;

	int width;
	int height;

	// The current REAL cursor position on the console
	int curX;
	int curY;

	int clipX0;
	int clipY0;
	int clipX1;
	int clipY1;

	bool raw_mode;

	//std::mutex lock;
	//std::atomic<bool> getLineStarted;

};

} // namespace bbs

#endif // BBS_CONSOLE_H
