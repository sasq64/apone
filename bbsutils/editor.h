#ifndef BBS_EDITOR_H
#define BBS_EDITOR_H

#include "console.h"

namespace bbs {

class Editor {
};

class LineEditor : public Editor {
public:
	LineEditor(Console &console, int width = 0);
	int update(int msec);
	bool isDone();
	std::string getResult();
	std::wstring getWResult();
	void refresh();

	void setString(const std::string &text);
	void setString(const std::wstring &text);
	void setXY(int x, int y);
	void setCursor(int pos);
	void setWidth(int w) { width = w; }

	int getCursor() { return xpos; }
	int getLength() { return line.length(); }

protected:
	Console &console;
	std::wstring line;

	int startX;
	int startY;
	int fg;
	int bg;

	int width;
	int maxlen;

	bool password;
	std::string filter;

	int xpos;
	int xoffset;

};

class FullEditor : public Editor {
public:
	FullEditor(Console &console);
	int update(int msec);
	std::string getResult();
	void refresh();

	void setString(const std::string &text);
	void setString(const std::wstring &text);

protected:
	void redraw(bool lines, int cursor = -1);

	Console &console;

	int startX;
	int startY;
	int width;
	int height;

	int yscroll;

	int lineNo;
	std::vector<std::wstring> lines;
	std::unique_ptr<LineEditor> lineEd;
};

}

#endif // BBS_EDITOR_H