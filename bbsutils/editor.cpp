
#include "editor.h"

#include <coreutils/log.h>
#include <coreutils/utils.h>

namespace bbs {

using namespace std;
using namespace utils;

LineEditor::LineEditor(Console &console, int width) : console(console), width(width), xpos(0), xoffset(0) {
	startX = console.getCursorX();
	startY = console.getCursorY();
	fg = console.getFg();
	bg = console.getBg();
	if(this->width == 0)
		this->width = console.getWidth() - startX - 1;
	maxlen = width;
	xpos = 0;
	line = L"";
}

void LineEditor::setString(const std::string &text) {
	line = utf8_decode(text);
}

void LineEditor::setString(const std::wstring &text) {
	line = text;
	if(xpos > line.size())
		xpos = line.size();
	//refresh();
}

void LineEditor::setXY(int x, int y) {
	startX = x;
	startY = y;
	//refresh();
}

void LineEditor::setCursor(int pos) {
	xpos = pos;
	if(xpos > line.size())
		xpos = line.size();
	//refresh();
}



int LineEditor::update(int msec) {

	auto c = console.getKey(msec);
	auto lastLen = line.length();
	switch(c) {
	//case Console::KEY_TIMEOUT:
	//	return -1;
	//case Console::KEY_ENTER:
	//	return 0;
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
		line = L"";
		break;
	default:
		if(c < 0x10000) {
			line.insert(xpos, 1, c);
			xpos++;
		} else
			return c;
		break;
	}

	auto endX = startX + width;
	auto cursorX = startX + xpos - xoffset;
	auto dx = startX - cursorX;

	if(dx > 0) {
		xoffset -= dx;
		cursorX += dx;
		lastLen = -1;
	}
	dx = cursorX - endX;
	if(dx >= 0) {
		xoffset += dx;
		cursorX -= dx;
		lastLen = -1;
	}

	//if(line.length() != lastLen) {
		refresh();
	//}

	//console.moveCursor(cursorX, startY);
	return c;
}

void LineEditor::refresh() {

	auto endX = startX + width;
	auto cursorX = startX + xpos - xoffset;
	auto dx = startX - cursorX;

	if(dx > 0) {
		xoffset -= dx;
		cursorX += dx;
		//lastLen = -1;
	}
	dx = cursorX - endX;
	if(dx >= 0) {
		xoffset += dx;
		cursorX -= dx;
		//lastLen = -1;
	}

	console.fill(bg, startX, startY, width, 1);
	auto l = line.substr(xoffset, width);
	console.put(startX, startY, l);
	console.flush();
	console.moveCursor(cursorX, startY);
}

string LineEditor::getResult() {
	return utils::utf8_encode(line);
}

wstring LineEditor::getWResult() {
	return line;
}
//

FullEditor::FullEditor(Console &console) : console(console) {
	console.clear();
	console.moveCursor(0,0);
	lineEd = make_unique<LineEditor>(console);
	lineNo = 0;
	yscroll = 0;
	startX = 0;
	startY = 0;
	width = console.getWidth();
	//height = console.getHeight() - 2;
	height = console.getHeight()-1;

	lineEd->setXY(startX, startY);
	lineEd->setWidth(width);
	lines.push_back(L"");

	//console.fill(Console::RED, 0, 0, 2, 0);
	//console.fill(Console::RED, 19, 0, 2, 0);

	console.fill(Console::RED, 0, -1, 0, 1);
	console.put(1, -1, format("%02d/%02d", lineNo+1, lines.size()), Console::WHITE, Console::RED);
	console.flush();


}

/*
height = 4
0 2
1 3 
2 4
3 5

lineNo = 5

*/



void FullEditor::redraw(bool full, int cursor) {
	//if(y1 <= 0)
	//	y1 = lines.size() + y1;

	//if(y1 > lines.size()-1)
	//	y1 = lines.size()-1;
	auto startLine = lineNo-yscroll-1;
	if(startLine < 0) startLine = 0;

	if(lineNo-yscroll >= height) {
		yscroll = lineNo - height + 1;
		//y0 = 0;
		//y1 = height;
		startLine = 0;
		full = true;
		LOGD("yscroll %d", yscroll);
	} else if(lineNo-yscroll < 0) {
		yscroll = lineNo;
		//y0 = 0;
		//y1 = height;
		startLine = 0;
		full = true;
		LOGD("yscroll %d", yscroll);
	}
	if(full) {
		for(int i=startLine; i<height; i++) {
			console.fill(Console::BLACK, startX, i + startY, width, 1);
			if(i+yscroll >= lines.size())
				break;
			console.put(startX, i + startY, lines[i + yscroll].substr(0,width));
		}
	}
	console.flush();
	if(cursor >= 0) {
		lineEd->setXY(startX, lineNo + startY - yscroll);
		lineEd->setString(lines[lineNo]);
		lineEd->setCursor(cursor);
		lineEd->refresh();
	}
}

int FullEditor::update(int msec){
	auto xpos = lineEd->getCursor();
	auto rc = lineEd->update(msec);

	if(rc == Console::KEY_TIMEOUT)
		return rc;

	LOGD("update %d %d", xpos, rc);

	//auto line = lineEd->getWResult();

	switch(rc) {
	case Console::KEY_ENTER:
		{
			auto x = lineEd->getCursor();
			auto l = lineEd->getWResult();
			auto l0 = l.substr(0, x);
			auto l1 = l.substr(x);
			lines[lineNo] = l0;
			lineNo++;
			lines.insert(lines.begin() + lineNo, l1);
			redraw(true, 0);
		}
		break;
	case Console::KEY_BACKSPACE:
		if(xpos == 0 && lineNo > 0) {
			auto l = lineEd->getWResult();
			lines.erase(lines.begin() + lineNo);
			lineNo--;
			auto len = lines[lineNo].length();
			lines[lineNo] = lines[lineNo] + l;
			redraw(true, len);
			//console.fill(Console::BLACK, startX, startY + lines.size(), width, 1);
		}
		break;
	case Console::KEY_DELETE:
		if(xpos == lineEd->getLength() && lineNo < lines.size()-1) {

			auto l = lineEd->getWResult();
			lineEd->setString(l + lines[lineNo+1]);
			lines.erase(lines.begin() + lineNo + 1);
			redraw(true);
			//console.fill(Console::BLACK, startX, startY +lines.size(), width, 1);
		}
		break;
	case Console::KEY_LEFT:
		if(xpos == 0 && lineNo > 0) {
			lines[lineNo--] = lineEd->getWResult();
			redraw(false,lines[lineNo].length());
		}
		break;
	case Console::KEY_RIGHT:
		if(xpos == lineEd->getLength() && lineNo < lines.size()-1) {
			lines[lineNo++] = lineEd->getWResult();
			redraw(false,0);
		}
		break;
	case Console::KEY_UP:
		if(lineNo > 0) {
			lines[lineNo--] = lineEd->getWResult();
			redraw(false,0);
		}
		break;
	case Console::KEY_DOWN:
		if(lineNo < lines.size()-1) {
			lines[lineNo++] = lineEd->getWResult();
			redraw(false,0);
		}
		break;
	}

	console.put(1, -1, format("%02d/%02d", lineNo+1, lines.size()), Console::WHITE, Console::RED);
	console.flush();

	return rc;
}

std::string FullEditor::getResult(){
	auto text = join(lines, L"\n");
	return utf8_encode(text);
}

void FullEditor::setString(const std::string &text) {
	lines = split(utf8_decode(text), wstring(L"\n"));
}

void FullEditor::setString(const std::wstring &text) {
	lines = split(text, wstring(L"\n"));
}



void FullEditor::refresh(){
	redraw(true);
}



} // namespace bbs