#ifndef UTILS_H
#define UTILS_H

#include "vec.h"

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
#include <thread>
#include <atomic>

//#include <math.h>
//#ifndef M_PI
//#define M_PI 3.14159265358979323846
//#endif

//#include "utils/format.h"

#ifdef ANDROID
#define stol(x) atol(x.c_str())
#define stoll(x) atoll(x.c_str())
#define stod(x) atof(x.c_str())
#endif

namespace utils {

typedef unsigned int uint;

const char path_separator = '/';

std::string utf8_encode(const std::string &s);
std::string utf8_encode(const std::wstring &s);
std::wstring utf8_decode(const std::string &s);

template <typename T>
std::vector<T> split(const T &s, const T &delim = T(" "), int limit = 0) {
	std::vector<T> args;
	auto l = delim.length();
	if(l == 0) return args;
	int pos = 0;
	while(true) {
		auto newpos = s.find(delim, pos);
		if((limit && args.size() == limit) || newpos == std::string::npos) {
			args.push_back(s.substr(pos));
			break;
		}
		//LOGD("%d->%d = '''%s'''", pos, newpos, utils::utf8_encode(s.substr(pos, newpos-pos)));
		args.push_back(s.substr(pos, newpos-pos));
		pos = newpos + l;
	}

	return args;
}
template <typename T>
std::vector<T> split(const T &s, const char *delim, bool inc = false) {
	return split(s, std::string(delim), inc);
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

template<template <typename, typename> class Container, class V, class A>
V join(const Container<V, A> &strings, const char *separator) {
	V out;
	for(const auto &s : strings) {
		out += (s + separator);
	}
	return out;
}

std::string urlencode(const std::string &s, const std::string &chars);
std::string urldecode(const std::string &s, const std::string &chars);

void sleepms(uint ms);
uint64_t getms();
void makedir(const std::string &name);
void makedirs(const std::string &name);

bool endsWith(const std::string &name, const std::string &ext);
bool startsWith(const std::string &name, const std::string &pref);
void makeLower(std::string &s);
std::string toLower(const std::string &s);

std::string rstrip(const std::string &x, char c = ' ');
std::string lstrip(const std::string &x, char c = ' ');
//std::string wordwrap( std::string str, size_t width);
std::vector<std::string> text_wrap(const std::string &text, int width, int subseqWidth = -1);

std::string path_basename(const std::string &name);
std::string path_directory(const std::string &name);
std::string path_filename(const std::string &name);
std::string path_extension(const std::string &name);
std::string path_suffix(const std::string &name);
std::string path_prefix(const std::string &name);

bool isalpha(const std::string &s);

float clamp(float x, float a0 = 0.0, float a1 = 1.0);

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

struct asyncthread {
	asyncthread() : done(false) {}
	~asyncthread() {
		if(t.joinable())
			t.join();
	}
	std::thread t;
	std::atomic<bool> done;
};

//static std::vector<asyncthread> atlist;

void cleanup_async();

void run_async(std::function<void()> f);


};

#include "file.h"

#endif // UTILS_H
