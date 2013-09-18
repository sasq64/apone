#ifndef BBS_CONSOLE_H
#define BBS_CONSOLE_H


#include "terminal.h"

#include <coreutils/log.h>

#include <stdint.h>
#include <termios.h>
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

class LocalTerminal : public Terminal {
public:

	virtual void open() override {
		struct termios new_term_attr;
		// set the terminal to raw mode
		tcgetattr(fileno(stdin), &orig_term_attr);
		memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
		new_term_attr.c_lflag &= ~(ECHO|ICANON);
		new_term_attr.c_cc[VTIME] = 0;
		new_term_attr.c_cc[VMIN] = 0;
		tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
	}

	virtual void close() override {
		LOGD("Restoring terminal");
		tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
	}

	virtual int write(const std::vector<Char> &source, int len) { return fwrite(&source[0], 1, len, stdout); }
	virtual int read(std::vector<Char> &target, int len) { return fread(&target[0], 1, len, stdin); }
private:
	struct termios orig_term_attr;

};

extern LocalTerminal localTerminal;

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
		KEY_BACKSPACE = 256,
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

		KEY_UNKNOWN = 0xfffe,
		KEY_TIMEOUT = 0xffff
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

	Console(Terminal &terminal) : terminal(terminal), fgColor(WHITE), bgColor(BLACK), width(40), height(25), curX(0), curY(0) {
		terminal.open();
	}

	~Console() {
		terminal.close();
	}

	virtual int getKey(int timeout = -1);
	virtual void clear();
	virtual void put(int x, int y, const std::string &text, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void put(int x, int y, Char c, int fg = CURRENT_COLOR, int bg = CURRENT_COLOR);
	virtual void write(const std::string &text);
	virtual void setColor(int fg, int bg = BLACK);
	virtual void resize(int w, int h);
	virtual void flush();
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
	int getCursorX() const { return curX; }
	int getCursorY() const { return curY; }
	int getFg() const { return fgColor; }
	int getBg() const { return bgColor; }

protected:

	void shiftTiles(std::vector<Tile> &tiles, int dx, int dy);
	void clearTiles(std::vector<Tile> &tiles, int x0, int y0, int w, int h);

	// Functions that needs to be implemented by real console implementations

	virtual void impl_color(int fg, int bg) = 0;
	virtual void impl_gotoxy(int x, int y) = 0;
	virtual bool impl_scroll_screen(int dy) { return false; };
	virtual bool impl_scroll_line(int dx) { return false; };
	virtual int impl_handlekey() = 0;
	virtual void impl_clear() = 0;
	virtual void impl_translate(Char &c) {}


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

	//std::mutex lock;
	//std::atomic<bool> getLineStarted;

};

Console *createLocalConsole();

} // namespace bbs

#endif // BBS_CONSOLE_H