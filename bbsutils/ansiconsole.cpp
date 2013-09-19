#include <coreutils/log.h>
#include "ansiconsole.h"
#include <coreutils/utils.h>

#include <array>
#include <algorithm> 

LOGSPACE("utils");

namespace bbs {

using namespace std;


static uint8_t c64pal [] = {
	0xFF, 0xFF, 0xFF, // WHITE
	0x68, 0x37, 0x2B, // RED
	0x58, 0x8D, 0x43, // GREEN
	0x35, 0x28, 0x79, // BLUE
	0x6F, 0x4F, 0x25, // ORANGE
	0x00, 0x00, 0x00, // BLACK
	0x43, 0x39, 0x00, // BROWN
	0x9A, 0x67, 0x59, // LIGHT_READ
	0x44, 0x44, 0x44, // DARK_GREY
	0x6C, 0x6C, 0x6C, // GREY
	0x9A, 0xD2, 0x84, // LIGHT_GREEN
	0x6C, 0x5E, 0xB5, // LIGHT_BLUE
	0x95, 0x95, 0x95, // LIGHT_GREY
	0x6F, 0x3D, 0x86, // PURPLE
	0xB8, 0xC7, 0x6F, // YELLOW
	0x70, 0xA4, 0xB2, // CYAN
};

AnsiConsole::AnsiConsole(Terminal &terminal) : Console(terminal) {
	//resize(width, height);
	for(int i=0; i<16; i++) {
		auto *p = &c64pal[i*3];
		const string &s = utils::format("\x1b]4;%d;#%02x%02x%02x\x07", 160 + i, p[0], p[1], p[2]);
		//LOGD(s);
		outBuffer.insert(outBuffer.end(), s.begin(), s.end());
	}
	impl_clear();
	impl_color(fgColor, bgColor);
	impl_gotoxy(0,0);

	//impl_gotoxy(0,0);
	//flush();
};


void AnsiConsole::putChar(Char c) {

	if(c < 0x80)
		outBuffer.push_back(c);
	else if(c < 0x800) {
		outBuffer.push_back(0xC0 | (c >> 6));
		outBuffer.push_back(0x80 | (c & 0x3F));
		//ato l = outBuffer.size();
		//LOGD("Translated %02x to %02x %02x", c, (int)outBuffer[l-2], (int)outBuffer[l-1]);
	} else /*if (c < 0x10000) */ {
		outBuffer.push_back(0xE0 | (c >> 12));
		outBuffer.push_back(0x80 | (c >> 6));
		outBuffer.push_back(0x80 | (c & 0x3F));
	}
	curX++;
	if(curX >= width) {
		curX -= width;
		curY++;
	}
}

bool AnsiConsole::impl_scroll_screen(int dy) {
	const auto s = dy > 0 ? utils::format("\x1b[%dS",dy) : utils::format("\x1b[%dT", -dy);
	outBuffer.insert(outBuffer.end(), s.begin(), s.end());
	return true;
}

void AnsiConsole::impl_color(int fg, int bg) {

	//int af = ansiColors[fg];
	//int ab = ansiColors[bg];

	//LOGD("## BG %d\n", ab);
	//const string &s = utils::format("\x1b[%d;%d%sm", af + 30, ab + 40, hl ? ";1" : "");
	const auto s = utils::format("\x1b[38;5;%d;48;5;%dm", fg+160, bg+160);

	//uint8_t *fp = &c64pal[fg*3];
	//uint8_t *bp = &c64pal[bg*3];
	//const string &s = utils::format("\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm", fp[0], fp[1], fp[2], bp[0], bp[1], bp[2]);
	outBuffer.insert(outBuffer.end(), s.begin(), s.end());			
};

void AnsiConsole::impl_clear() {
	for(auto x : vector<uint8_t> { '\x1b', '[', '2', 'J' })
		outBuffer.push_back(x);
}


void AnsiConsole::impl_gotoxy(int x, int y) {
	// Not so smart for now
	//LOGD("gotoxy %d,%d", x,y);
	const auto s = utils::format("\x1b[%d;%dH", y+1, x+1);
	outBuffer.insert(outBuffer.end(), s.begin(), s.end());
	curX = x;
	curY = y;
}

int AnsiConsole::impl_handlekey() {
	auto c = inBuffer.front();
	if(c >= 0x80)
		return get_utf8();
	inBuffer.pop();
	if(c != 0x1b) {	
		LOGD("Normal key %d", (int)c);
		if(c == 13 || c == 10) {
			if(c == 13) {
				auto c2 = inBuffer.front();
				if(c2 == 10) {
					inBuffer.pop();
				}
			}
			return KEY_ENTER;
		} else if(c == 0x7f)
			return KEY_BACKSPACE;
		else if(c == 0x7e)
			return KEY_DELETE;
		return c;
	} else {
		if(inBuffer.size() > 0) {
			auto c2 = inBuffer.front();
			inBuffer.pop();
			auto c3 = inBuffer.front();
			inBuffer.pop();

			LOGD("ESCAPE key %02x %02x %02x", c, c2, c3);

			if(c2 == 0x5b || c2 == 0x4f) {
				switch(c3) {
				case 0x50:
					return KEY_F1;
				case 0x51:
					return KEY_F2;
				case 0x52:
					return KEY_F3;
				case 0x53:
					return KEY_F4;
				case 0x31:
					if(inBuffer.size() >= 2) {
						auto c4 = inBuffer.front();
						inBuffer.pop();
						auto c5 = inBuffer.front();
						inBuffer.pop();
						if(c5 == 126) {
							switch(c4) {
							case '5':
								return KEY_F5;
							case '7':
								return KEY_F6;
							case '8':
								return KEY_F7;
							case '9':
								return KEY_F8;
							}
						}
					}
					break;
				case 0x33:
					if(!inBuffer.empty() && inBuffer.front() == 126)
						inBuffer.pop();
					return KEY_DELETE;
				case 0x48:
					return KEY_HOME;
				case 0x46:
					return KEY_END;
				case 0x44:
					return KEY_LEFT;
				case 0x43:
					return KEY_RIGHT;
				case 0x41:
					return KEY_UP;
				case 0x42:
					return KEY_DOWN;
				case 0x35:
					inBuffer.pop();
					return KEY_PAGEUP;
				case 0x36:
					inBuffer.pop();
					return KEY_PAGEDOWN;
					
				}
			}
		}
		return c;
	}
}

}
