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

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <initializer_list>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

std::string rstrip(const std::string &x, char c = ' ');

std::string path_basename(const std::string &name);
std::string path_directory(const std::string &name);
std::string path_filename(const std::string &name);
std::string path_extention(const std::string &name);
std::string path_suffix(const std::string &name);
std::string path_prefix(const std::string &name);

std::string utf8_encode(const std::string &s);
std::string utf8_encode(const std::wstring &s);

// Vectors

template <class T> struct vec {
	vec() : x(0), y(0) {}
	vec(T x, T y) : x(x), y(y) {}

	vec operator+(const vec &v) const {
		return vec(x + v.x, y + v.y);
	}

	vec operator+(std::initializer_list<T> il) const {
		return vec(x + 0, y + 0);
	}

	vec operator-(const vec &v) const {
		return vec(x - v.x, y - v.y);
	}

	vec operator/(T n) const {
		return vec(x / n, y / n);
	}

	vec operator*(T n) const {
		return vec(x * n, y * n);
	}

	T angle() {
		T l = sqrt(x * x + y * y);
		T a = acos(x / l);
		if(y < 0) a = (M_PI - a) + M_PI;
			return a;
	}

	T dist2() {
		return x * x + y * y;
	}

	T dist() {
		return sqrt(x * x + y * y);
	}

	T x;
	T y;
};


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

// make_uniqie by STL


template<class T> struct _Unique_if {
    typedef std::unique_ptr<T> _Single_object;
};

template<class T> struct _Unique_if<T[]> {
    typedef std::unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N> struct _Unique_if<T[N]> {
    typedef void _Known_bound;
};

template<class T, class... Args>
    typename _Unique_if<T>::_Single_object
    make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

template<class T>
    typename _Unique_if<T>::_Unknown_bound
    make_unique(size_t n) {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[n]());
    }

template<class T, class... Args>
    typename _Unique_if<T>::_Known_bound
    make_unique(Args&&...) = delete;


};

#endif // UTILS_H
