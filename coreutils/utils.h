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
#include <stdexcept>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//#include "utils/format.h"

namespace utils {

typedef unsigned int uint;

const char path_separator = '/';

std::string utf8_encode(const std::string &s);
std::string utf8_encode(const std::wstring &s);
std::wstring utf8_decode(const std::string &s);

template <typename T>
std::vector<T> split(const T &s, const T &delim = T(" ")) {
	std::vector<T> args;
	auto l = delim.length();
	if(l == 0) return args;
	int pos = 0;
	while(true) {
		auto newpos = s.find(delim, pos);
		if(newpos == std::string::npos) {
			args.push_back(s.substr(pos));
			break;
		}
		LOGD("%d->%d = '''%s'''", pos, newpos, utils::utf8_encode(s.substr(pos, newpos-pos)));
		args.push_back(s.substr(pos, newpos-pos));
		pos = newpos + l;
	}

	return args;
}

template<template <typename, typename> class Container, class V, class A>
V join(const Container<V, A> &strings, const V &separator) {
	V out;
	for(const auto &s : strings) {
		out += (s + separator);
	}
	return out;
}

template<template <typename, typename> class Container, class V, class A>
V join(const Container<V, A> &strings, const wchar_t *separator) {
	V out;
	for(const auto &s : strings) {
		out += (s + separator);
	}
	return out;
}


std::string urlencode(const std::string &s, const std::string &chars);
std::string urldecode(const std::string &s, const std::string &chars);

void sleepms(uint ms);
void makedir(const std::string &name);
void makedirs(const std::string &name);

bool endsWith(const std::string &name, const std::string &ext);
void makeLower(std::string &s);

std::string rstrip(const std::string &x, char c = ' ');
std::string lstrip(const std::string &x, char c = ' ');
//std::string wordwrap( std::string str, size_t width);
std::vector<std::string> text_wrap(const std::string &text, int width, int subseqWidth = -1);

std::string path_basename(const std::string &name);
std::string path_directory(const std::string &name);
std::string path_filename(const std::string &name);
std::string path_extention(const std::string &name);
std::string path_suffix(const std::string &name);
std::string path_prefix(const std::string &name);

// Vectors

template <class T> struct vec2 {
	vec2() : x(0), y(0) {}
	vec2(T x, T y) : x(x), y(y) {}
	vec2(std::pair<T, T> pair) : x(pair.first), y(pair.second) {}

	vec2(std::initializer_list<T> &il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
	}

	vec2 operator+(const vec2 &v) const {
		return vec2(x + v.x, y + v.y);
	}

	vec2 operator+(const T &i) const {
		return vec2(x + i, y + i);
	}

	vec2 operator+(std::initializer_list<T> il) const {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		return vec2(x + xa, y + ya);
	}

	vec2 operator+=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		x += xa;
		y += ya;
		return *this;
	}

	T operator[](const int &i) {
		switch(i) {
		case 0:
			return x;
		case 1:
			return y;
		}
		throw std::out_of_range("Only 0 or 1 are valid indexes");
	}

	vec2 operator-(const vec2 &v) const {
		return vec2(x - v.x, y - v.y);
	}

	vec2 operator/(T n) const {
		return vec2(x / n, y / n);
	}

	vec2 operator*(T n) const {
		return vec2(x * n, y * n);
	}

	vec2 operator*(const vec2 &v) const {
		return vec2(x * v.x, y * v.y);
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

typedef vec2<float> vec2f;
typedef vec2<int> vec2i;

template <typename T> vec2<T> cossin(const vec2<T> &v) {
	return vec2<T>(cos(v.x), sin(v.y));
}

template <typename T> vec2<T> sin(const vec2<T> &v) {
	return vec2<T>(sinf(v.x), sinf(v.y));
}

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
