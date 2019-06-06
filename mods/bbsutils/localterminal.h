#pragma once

#include "ansiconsole.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

namespace bbs {

struct LocalTerminal : public Terminal
{
    void open()
    {
        struct termios new_term_attr;
        // set the terminal to raw mode
        LOGD("Setting RAW mode");
        if (tcgetattr(fileno(stdin), &orig_term_attr) < 0)
            LOGD("FAIL");
        memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
        new_term_attr.c_lflag &= ~(ECHO | ICANON);
        new_term_attr.c_cc[VTIME] = 0;
        new_term_attr.c_cc[VMIN] = 0;
        if (tcsetattr(fileno(stdin), TCSANOW, &new_term_attr) < 0)
            LOGD("FAIL");

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) < 0)
            LOGD("IOCTL FAIL");

        setvbuf(stdout, NULL, _IONBF, 0);
    }

    int getWidth() const { return ws.ws_col; }

    int getHeight() const { return ws.ws_row; }

    void close()
    {
        LOGD("Restoring terminal");
        tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
    }

    int write(const std::vector<Char>& source, int len)
    {
        return ::write(fileno(stdout), &source[0], len);
    }

    int read(std::vector<Char>& target, int len)
    {
        return ::read(fileno(stdin), &target[0], len);
    }
private:
    struct termios orig_term_attr;
    struct winsize ws;
};

} // namespace bbs
