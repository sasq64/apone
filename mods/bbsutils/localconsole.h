#ifndef LOCALCONSOLE_H
#define LOCALCONSOLE_H

#include "console.h"

//#ifdef LINUX

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace bbs {

class LocalTerminal : public Terminal {
public:

	virtual void open() override;
	virtual int getWidth() const override;
	virtual int getHeight() const override;

	virtual void close() override;

	virtual int write(const std::vector<Char> &source, int len) override;
	virtual int read(std::vector<Char> &target, int len) override;
private:
	struct termios orig_term_attr;
	struct winsize ws;
	

};
extern LocalTerminal localTerminal;

}
//#endif

#endif // LOCALCONSOLE_H

