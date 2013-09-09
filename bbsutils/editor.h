#ifndef BBS_EDITOR_H
#define BBS_EDITOR_H

#include "console.h"

namespace bbs {

class Editor {
};

class LineEditor : public Editor {
public:
	LineEditor(Console &console);
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
	int x;
};

}

#endif // BBS_EDITOR_H