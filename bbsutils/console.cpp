#include "console.h"

#include "editor.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>

#include <array>
#include <algorithm>
 

LOGSPACE("utils");

namespace bbs {

using namespace std;

#ifdef LOCAL_TERMINAL
//LocalTerminal localTerminal;
#endif

void Console::clear() {
	Tile t { 0x20, WHITE, BLACK };
	std::fill(begin(grid), end(grid), t);
	std::fill(begin(oldGrid), end(oldGrid), t);
	impl_clear();
	//curX = curY = 0;
}

void Console::refresh() {

	Tile t { 0, -1, -1 };
	std::fill(begin(oldGrid), end(oldGrid), t);
	impl_clear();
	curX = curY = 0;
	flush();
}

void Console::fill(int bg, int x, int y, int w, int h) {

	if(x < 0) x = width+x;
	if(y < 0) y = height+y;
	if(w <= 0) w = width+w;
	if(h <= 0) h = height+h;

	if(bg == CURRENT_COLOR)
		bg = bgColor;
	for(int yy = y; yy < y + h; yy++)
		for(int xx = x; xx < x + w; xx++) {
			auto &t = grid[xx + width * yy];
			if(fgColor >= 0) t.fg = fgColor;
			if(bg >= 0) t.bg = bg;
			t.c = 0x20;
		}
}

static const u_int32_t offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/* returns length of next utf-8 sequence */
int u8_seqlen(char *s)
{
    return trailingBytesForUTF8[(unsigned int)(unsigned char)s[0]] + 1;
}

/* conversions without error checking
   only works for valid UTF-8, i.e. no 5- or 6-byte sequences
   srcsz = source size in bytes, or -1 if 0-terminated
   sz = dest size in # of wide characters

   returns # characters converted
   dest will always be L'\0'-terminated, even if there isn't enough room
   for all the characters.
   if sz = srcsz+1 (i.e. 4*srcsz+4 bytes), there will always be enough space.
*/
int u8_to_ucs(const char *src, uint32_t *dest, int sz)
{
    u_int32_t ch;
    int nb;
    int i=0;

    while (i < sz-1) {
        nb = trailingBytesForUTF8[(unsigned char)*src];
        if (*src == 0)
        	break;
        ch = 0;
        switch (nb) {
            /* these fall through deliberately */
        case 3: ch += (unsigned char)*src++; ch <<= 6;
        case 2: ch += (unsigned char)*src++; ch <<= 6;
        case 1: ch += (unsigned char)*src++; ch <<= 6;
        case 0: ch += (unsigned char)*src++;
        }
        ch -= offsetsFromUTF8[nb];
        dest[i++] = ch;
    }

    dest[i] = 0;
    return i;
}

int Console::get_utf8(){

	int nb = trailingBytesForUTF8[inBuffer.front()];

	LOGD("Utf8 %02x -> %d bytes, vs %d bytes", inBuffer.front(),nb, inBuffer.size());


	int ch = 0;
	switch (nb) {
	case 3: ch += inBuffer.front(); inBuffer.pop(); ch <<= 6;
	case 2: ch += inBuffer.front(); inBuffer.pop(); ch <<= 6;
	case 1: ch += inBuffer.front(); inBuffer.pop(); ch <<= 6;
	case 0: ch += inBuffer.front(); inBuffer.pop();
	}
	ch -= offsetsFromUTF8[nb];
	LOGD("Got %04x", ch);
	return ch;
}


void Console::put(int x, int y, Char c, int fg, int bg) {
	if(x < 0) x = width+x;
	if(y < 0) y = height+y;

	if(y >= clipY1 || y < clipY0 || x >= clipX1 || x < clipX0)
		return;

	auto &t = grid[x + y * width];
	t.c = c;
	impl_translate(t.c);

	if(fg == CURRENT_COLOR)
		fg = fgColor;
	if(bg == CURRENT_COLOR)
		bg = bgColor;

	if(fg >= 0)
		t.fg = fg;
	if(bg >= 0)
		t.bg = bg;

}

void Console::put(int x, int y, const string &text, int fg, int bg) {

	if(x < 0) x = width+x;
	if(y < 0) y = height+y;

	if(y >= clipY1 || y < clipY0)
		return;

    vector<uint32_t> output(128);
    int l = u8_to_ucs(text.c_str(), &output[0], 128);

	//for(int i=0; i<(int)text.length(); i++) {
    for(int i=0; i<l; i++) {

    	if(x+i < clipX0)
    		continue;
		if(x+i >= clipX1)
			return;

		auto &t = grid[(x+i) + y * width];
		t.c = output[i];
		impl_translate(t.c);
		//LOGD("Putting %04x as %04x", output[i], t.c);

		if(fg == CURRENT_COLOR)
			fg = fgColor;
		if(bg == CURRENT_COLOR)
			bg = bgColor;

		if(fg >= 0)
			t.fg = fg;
		if(bg >= 0)
			t.bg = bg;
	}
}

void Console::put(int x, int y, const wstring &text, int fg, int bg) {

	if(x < 0) x = width+x;
	if(y < 0) y = height+y;

	if(y >= clipY1 || y < clipY0)
		return;
	
	for(int i=0; i<(int)text.length(); i++) {
  
    	if(x+i < clipX0)
    		continue;
		if(x+i >= clipX1)
			return;

		auto &t = grid[(x+i) + y * width];
		t.c = text[i];
		impl_translate(t.c);
		//LOGD("Putting %04x as %04x", output[i], t.c);

		if(fg == CURRENT_COLOR)
			fg = fgColor;
		if(bg == CURRENT_COLOR)
			bg = bgColor;

		if(fg >= 0)
			t.fg = fg;
		if(bg >= 0)
			t.bg = bg;
	}
}

void Console::resize(int w, int h) {
	width = w;
	height = h;
	clipX0 = clipY0 = 0;
	clipX1 = w;
	clipY1 = h;
	LOGD("Resize");
	grid.resize(w*h);
	oldGrid.resize(w*h);
	clear();
}

void Console::flush(bool restoreCursor) {

	auto w = terminal.getWidth();
	auto h = terminal.getHeight();
	if((w > 0 && w != width) || (h > 0 && h != height)) {
		resize(w, h);
	}

	auto saveX = curX;
	auto saveY = curY;

	int saveFg = fgColor;
	int saveBg = bgColor;

	//auto curFg = fgColor;
	//auto curBg = bgColor;

	// TODO: Try this from clean oldGrid and clear before if more effecient

	for(int y = 0; y<height; y++) {
		for(int x = 0; x<width; x++) {
			auto &t0 = oldGrid[x+y*width];
			auto &t1 = grid[x+y*width];
			if(t0 != t1) {
				if(curY != y or curX != x) {
					impl_gotoxy(x, y);
				}
				if(t1.fg != fgColor || t1.bg != bgColor) {
					impl_color(t1.fg, t1.bg);
					fgColor = t1.fg;
					bgColor = t1.bg;
				}
				putChar(t1.c);
				t0 = t1;
			}
			/*if(outBuffer.size() > 0) {
				terminal.write(outBuffer, outBuffer.size());
				outBuffer.resize(0);
				utils::sleepms(20);
			}*/

		}
	}

	if(saveFg != fgColor || saveBg != bgColor) {
		fgColor = saveFg;
		bgColor = saveBg;
		if(fgColor >= 0 && bgColor >= 0) {
			LOGD("Restoring color to %d %d", fgColor, bgColor);
			impl_color(fgColor, bgColor);
		}
	}

	//LOGD("Restorting cursor");
	if(restoreCursor)
		impl_gotoxy(saveX, saveY);

	if(outBuffer.size() > 0) {
		LOGV("OUTBYTES: [%02x]", outBuffer);
		terminal.write(outBuffer, outBuffer.size());
		outBuffer.resize(0);
	}
}

void Console::putChar(Char c) {
	outBuffer.push_back(c & 0xff);
	curX++;
	if(curX >= width) {
		curX -= width;
		curY++;
	}
}

void Console::setColor(int fg, int bg) {
	fgColor = fg;
	bgColor = bg;
	impl_color(fg, bg);
	if(outBuffer.size() > 0) {
		terminal.write(outBuffer, outBuffer.size());
		outBuffer.resize(0);
	}
}


void Console::moveCursor(int x, int y) {

	if(x < 0) x = width+x;
	if(y < 0) y = height+y;

	if(curX == x && curY == y)
		return;

	impl_gotoxy(x, y);
	if(outBuffer.size() > 0) {
		terminal.write(outBuffer, outBuffer.size());
		outBuffer.resize(0);
	}
	//curX = x;
	//curY = y;
}

void Console::write(const std::string &text) {

	auto x = curX;
	auto y = curY;

	//LOGD("Putting %s to %d,%d", text, x, y);

	while(y >= height) {
		scrollScreen(1);
		y--;
	}
	auto spaces = 0;
	for(auto i=0; i<(int)text.length(); i++) {
	//for(const auto &c : text) {
		char c;
		if(spaces) {
			spaces--;
			i--;
			c = ' ';
		} else {
			c = text[i];
			if(c == '\t') {
				spaces = 4;
				continue;
			}
		}


		if(x >= width || c == 0xa || c == 0xd) {
			x = 0;
			y++;
			if(y >= height) {
				scrollScreen(1);
				y--;
			}

			if(c == 0xd) {
				if(text[i+1] == 0xa)
					i++;
				c = 0xa;
			}

			if(c == 0xa)
				continue;
		}

		auto &t = grid[x + y * width];
		x++;
		//LOGD("put to %d %d",x+i,y);	
		t.c = (Char)(c & 0xff);
		impl_translate(t.c);
		if(fgColor >= 0)
			t.fg = fgColor;
		if(bgColor >= 0)
			t.bg = bgColor;
		//flush();
		//this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	LOGD("%d/%d", curX, curY);
	flush();
	moveCursor(x, y);
	LOGD("%d/%d", curX, curY);
}

int Console::getKey(int timeout) {

	std::chrono::milliseconds ms { 100 };

	std::vector<uint8_t> temp;
	temp.reserve(16);

	while(true) {
		auto rc = terminal.read(temp, 16);
		if(rc > 0) {
			//LOGD("Got %d bytes", rc);
			for(int i=0; i<rc; i++) {
				//LOGD("Pushing %d", (int)temp[i]);
				inBuffer.push(temp[i]);
			}
		}
		if(inBuffer.size() > 0) {
			//LOGD("Size %d", inBuffer.size());
			return impl_handlekey();
		}

		std::this_thread::sleep_for(ms);
		if(timeout >= 0) {
			timeout -= 100;
			if(timeout < 0)
				return KEY_TIMEOUT;
		}
	}
}

std::string Console::getLine(int maxlen) {
	auto lineEd = utils::make_unique<LineEditor>(*this, maxlen);
	while(lineEd->update(500) != KEY_ENTER);
	if(maxlen == 0) {
		moveCursor(0, curY+1);
	}
	return lineEd->getResult();
}

std::string Console::getPassword(int maxlen) {
	auto lineEd = utils::make_unique<LineEditor>(*this, [](int c) -> int {
		return c >= 0x20 && c < 0x7f ? '*' : c;
	}, maxlen);
	while(lineEd->update(500) != KEY_ENTER);
	return lineEd->getResult();
}

// Shift all tiles
void Console::shiftTiles(vector<Tile> &tiles, int dx, int dy) {
	auto tempTiles = tiles;
	for(int y = 0; y<height; y++) {
		for(int x = 0; x<width; x++) {
			int tx = x - dx;
			int ty = y - dy;
			if(tx >= 0 && ty >= 0 && tx < width && ty < height)
				tiles[tx+ty*width] = tempTiles[x+y*width];
		}
	}
}

void Console::clearTiles(vector<Tile> &tiles, int x0, int y0, int w, int h) {
	auto x1 = x0 + w;
	auto y1 = y0 + h;
	for(int y = y0; y<y1; y++) {
		for(int x = x0; x<x1; x++) {
			Tile &t = tiles[x+y*width];
			t.c = 0x20;
			t.fg = WHITE;
			t.bg = BLACK;
		}
	}
}

void Console::scrollScreen(int dy) {

	shiftTiles(grid, 0, dy);
	clearTiles(grid, 0, height-dy, width, dy);
	if(impl_scroll_screen(dy)) {
		shiftTiles(oldGrid, 0, dy);
		clearTiles(oldGrid, 0, height-dy, width, dy);
	}
	flush();
}


#ifdef LOCAL_TERMINAL
Console *createLocalConsole() {
	return nullptr;//new AnsiConsole(localTerminal);
}
#endif
}

#ifdef UNIT_TEST

#include "catch.hpp"
#include <sys/time.h>

using namespace bbs;

class TestTerminal : public Terminal {
public:
	virtual int write(const std::vector<Char> &source, int len) { 
		for(int i=0; i<len; i++)
			outBuffer.push_back(source[i]);
		return len;
	}
	virtual int read(std::vector<Char> &target, int len) { 
		int rc = -1;//outBuffer.size();
		//target.insert(target.back, outBuffer.begin(), outBuffer.end());
		//outBuffer.resize(0);
		return rc;
	}
	std::vector<Char> outBuffer;


};
/*
TEST_CASE("console::basic", "Console") {

	TestTerminal terminal;
	PetsciiConsole console { terminal };

	//1b 5b 32 4a 1b 5b 32 3b 32 48  74 65 73 74 69 6e 67
	console.setFg(Console::WHITE);
	console.setBg(Console::BLACK);
	console.put(37,1, "abcdefghijk");
	console.put(0, 3, "ABCDEFGH");
	console.flush();
	string s = utils::format("[%02x]\n", terminal.outBuffer);
	printf(s.c_str());


	REQUIRE(true);

}
*/
#endif