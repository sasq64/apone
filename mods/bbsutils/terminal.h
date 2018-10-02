#pragma once

#include <stdint.h>
#include <string>
#include <vector>

namespace bbs {

class Terminal
{

public:
    typedef uint8_t Char;

    virtual int write(const std::vector<Char>& source, int len) = 0;
    virtual int read(std::vector<Char>& target, int len) = 0;

    virtual void open() {}
    virtual void close() {}

    virtual int getWidth() const { return -1; }
    virtual int getHeight() const { return -1; }
    virtual std::string getTermType() const { return ""; }
};

} // namespace bbs
