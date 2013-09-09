
#include "editor.h"

#include <coreutils/log.h>

namespace bbs {

using namespace std;

LineEditor::LineEditor(Console &console) : console(console) {
	startX = console.getCursorX();
	startY = console.getCursorY();
	fg = console.getFg();
	bg = console.getBg();
	x = 0;
	line = "";
}

int LineEditor::update(int msec) {

	auto c = console.getKey(msec);
	auto lastLen = line.length();
	switch(c) {
	case Console::KEY_ENTER:
		return 0;
	case Console::KEY_BACKSPACE:
		if(x > 0) {
			x--;				
			line.erase(x, 1);
		}
		break;
	case Console::KEY_DELETE:
		if(x < (int)line.length()) {
			line.erase(x, 1);
		}
		break;
	case Console::KEY_LEFT:
		if(x > 0)
			x--;
		break;
	case Console::KEY_HOME:
		x = 0;
		break;
	case Console::KEY_END:
		x = line.length();
		break;
	case Console::KEY_RIGHT:
		if(x < (int)line.length())
			x++;
		break;
	default:
		if(c < 256) {
			line.insert(x, 1, c);
			x++;
		} else
			return c;
		break;
	}
	if(line.length() != lastLen) {
		refresh();
	}
	console.moveCursor(startX + x, startY);
	return -1;
}

void LineEditor::refresh() {
	console.put(startX, startY, line);
	//if(lastLen > line.length())
		console.put(startX+line.length(), startY, " ");
	LOGD("Line now '%s'", line);
	console.flush();

}

string LineEditor::getResult() {
	return line;
}

} // namespace bbs