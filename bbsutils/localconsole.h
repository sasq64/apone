#ifndef LOCALCONSOLE_H
#define LOCALCONSOLE_H

#include "console.h"

#ifdef LINUX

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

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

		ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);

	}

	virtual int getWidth() const override {
		return ws.ws_col;
	}

	virtual int getHeight() const override {
		return ws.ws_row;
	}

	virtual void close() override {
		LOGD("Restoring terminal");
		tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
	}

	virtual int write(const std::vector<Char> &source, int len) { return fwrite(&source[0], 1, len, stdout); }
	virtual int read(std::vector<Char> &target, int len) { return fread(&target[0], 1, len, stdin); }

private:
	struct termios orig_term_attr;
	struct winsize ws;
	

};
extern LocalTerminal localTerminal;

}
#endif

#endif // LOCALCONSOLE_H

