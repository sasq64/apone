
#include "editor.h"

#include <coreutils/log.h>

namespace bbs {

using namespace std;

LineEditor::LineEditor(Console &console, int width) : console(console), width(width), xpos(0), xoffset(0) {
	startX = console.getCursorX();
	startY = console.getCursorY();
	fg = console.getFg();
	bg = console.getBg();
	if(this->width == 0)
		this->width = console.getWidth() - startX - 1;
	maxlen = width;
	xpos = 0;
	line = "";
}

int LineEditor::update(int msec) {

	auto c = console.getKey(msec);
	auto lastLen = line.length();
	switch(c) {
	case Console::KEY_TIMEOUT:
		return -1;
	case Console::KEY_ENTER:
		return 0;
	case Console::KEY_BACKSPACE:
		if(xpos > 0) {
			xpos--;				
			line.erase(xpos, 1);
		}
		break;
	case Console::KEY_DELETE:
		if(xpos < (int)line.length()) {
			line.erase(xpos, 1);
		}
		break;
	case Console::KEY_LEFT:
		if(xpos > 0)
			xpos--;
		break;
	case Console::KEY_HOME:
		xpos = 0;
		break;
	case Console::KEY_END:
		xpos = line.length();
		break;
	case Console::KEY_RIGHT:
		if(xpos < (int)line.length())
			xpos++;
		break;
	case Console::KEY_ESCAPE:
		xpos = 0;
		line = "";
		break;
	default:
		if(c < 256) {
			line.insert(xpos, 1, c);
			xpos++;
		} else
			return c;
		break;
	}

	auto endX = startX + width;
	auto cursorX = startX + xpos - xoffset;
	while(cursorX < startX) {
		xoffset--;
		cursorX++;
		lastLen = -1;
	}
	while(cursorX >= endX) {
		xoffset++;
		cursorX--;
		lastLen = -1;
	}

	if(line.length() != lastLen) {
		refresh();
	}

	//int cursorx = startX + x;
	//if(cursorx > maxCursor)
	//	cursorx = maxCursor;


	console.moveCursor(cursorX, startY);
	return -1;
}

void LineEditor::refresh() {
	console.fill(bg, startX, startY, width, 1);
	auto l = line.substr(xoffset, width) + " ";
	console.put(startX, startY, l);
	//if(lastLen > line.length())
	//	console.put(startX+line.length(), startY, " ");
	console.flush();

}

string LineEditor::getResult() {
	return line;
}

} // namespace bbs