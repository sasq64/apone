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
	void refresh();
protected:
	Console &console;
	std::string line;

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

}

#endif // BBS_EDITOR_H