#ifndef UTILS_H
#define UTILS_H

#include "file.h"

#include <sys/stat.h>
#include <stdint.h>
#include <typeinfo>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

//#include "utils/format.h"

namespace utils {

typedef unsigned int uint;

const char path_separator = '/';


class StringTokenizer {
public:
	StringTokenizer(const std::string &s, const std::string &delim);
	int noParts() { return args.size(); }
	const std::string &getString(int no) { return args[no]; }
	const char getDelim(int no) { return delims[no]; }
private:
	std::vector<std::string> args;
	std::vector<char> delims;
};

std::vector<std::string> split(const std::string &s, const std::string &delim = " ");

std::string urlencode(const std::string &s, const std::string &chars);
std::string urldecode(const std::string &s, const std::string &chars);

void sleepms(uint ms);
void makedir(const std::string &name);
void makedirs(const std::string &name);

bool endsWith(const std::string &name, const std::string &ext);
void makeLower(std::string &s);

std::string path_basename(const std::string &name);
std::string path_directory(const std::string &name);
std::string path_filename(const std::string &name);
std::string path_extention(const std::string &name);
std::string path_suffix(const std::string &name);
std::string path_prefix(const std::string &name);

std::string utf8_encode(const std::string &s);

// SLICE

template <class InputIterator> class slice {
public:
	slice(InputIterator start, InputIterator stop) : start(start), stop(stop) {}

	InputIterator begin() const {
		return start;
	}

	InputIterator end() const {
		return stop;
		//return const_iterator(*this, end);
	}

private:
	InputIterator start;
	InputIterator stop;

};

template <class T> slice<typename T::const_iterator> make_slice(T &vec, int start, int len) {
	return slice<typename T::const_iterator>(vec.begin() + start, vec.begin() + start + len);
}

};

#endif // UTILS_H
