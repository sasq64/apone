#ifndef UTILS_H
#define UTILS_H

//#include "vec.h"

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

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef ANDROID

// Android (for complicated reasons) misses some standard string conversion functions

namespace std {

long stol(const std::string &x);
long long stoll(const std::string &x);
long long stod(const std::string &x);

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

}
#endif

namespace utils {

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
	bool crlf = (delim.size() == 1 && delim[0] == 10);
	while(true) {
		auto newpos = s.find(delim, pos);
		if((limit && args.size() == limit) || newpos == std::string::npos) {
			args.push_back(s.substr(pos));
			break;
		}
		//LOGD("%d->%d = '''%s'''", pos, newpos, utils::utf8_encode(s.substr(pos, newpos-pos)));
		args.push_back(s.substr(pos, newpos-pos));
		pos = newpos + l;
		if(crlf && pos < s.length() && s[pos] == 13) pos++;
	}

	return args;
}
template <typename T>
std::vector<T> split(const T &s, const char *delim, int limit = 0) {
	return split(s, std::string(delim), limit);
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

void replace_char(std::string &s, char c, char r);
void replace_char(char *s, char c, char r);

std::string urlencode(const std::string &s, const std::string &chars);
std::string urldecode(const std::string &s, const std::string &chars);

std::string htmldecode(const std::string &source);

void sleepms(unsigned ms);
uint64_t getms();
void makedir(const std::string &name);
void makedirs(const std::string &name);

bool endsWith(const std::string &name, const std::string &ext);
bool startsWith(const std::string &name, const std::string &pref);
void makeLower(std::string &s);
std::string toLower(const std::string &s);

std::string rstrip(const std::string &x, char c = ' ');
std::string lstrip(const std::string &x, char c = ' ');
std::string lrstrip(const std::string &x, char c = ' ');
//std::string wordwrap( std::string str, size_t width);
std::vector<std::string> text_wrap(const std::string &text, int width, int initialWidth = -1);

std::string path_basename(const std::string &name);
std::string path_directory(const std::string &name);
std::string path_filename(const std::string &name);
std::string path_extension(const std::string &name);
std::string path_suffix(const std::string &name);
std::string path_prefix(const std::string &name);

std::string current_exe_path();

bool isalpha(const std::string &s);

float clamp(float x, float a0 = 0.0, float a1 = 1.0);

void schedule_callback(std::function<void()> f);
void perform_callbacks();

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


template <typename T> struct _ct {

	_ct(const T &to) : to(to) {}
	T to;

	struct const_iterator  {
		const_iterator(const T& index) : index(index) {}
		const_iterator(const const_iterator& rhs) : index(rhs.index) {}

		bool operator!= (const const_iterator& other) const {
			return index != other.index;
		}
 
		int32_t operator* () const {
			return index;
		}
 
		const const_iterator& operator++ () {
			index++;
			return *this;
		}
		int32_t index;
	};

	const_iterator begin() const { return const_iterator(0); }
	const_iterator end() const { return const_iterator(to); }
};

template <typename T> struct _cf {

	_cf(const T &from) : from(from) {}
	T from;

	struct const_iterator  {
		const_iterator(const T& index) : index(index) {}
		const_iterator(const const_iterator& rhs) : index(rhs.index) {}

		bool operator!= (const const_iterator& other) const {
			return index != other.index;
		}
 
		T operator* () const {
			return index;
		}
 
		const const_iterator& operator++ () {
			index--;
			return *this;
		}
		T index;
	};

	const_iterator begin() const { return const_iterator(from-1); }
	const_iterator end() const { return const_iterator(-1); }
};

template <typename T> _ct<T> count_to(const T &t) { return _ct<T>(t); }
template <typename T> _cf<T> count_from(const T &f) { return _cf<T>(f); }

// make_unique by STL


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
