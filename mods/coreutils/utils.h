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
#include <cstring>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <initializer_list>
#include <stdexcept>
#include <atomic>
#include <functional>

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if (defined ANDROID) || (defined WIN32)

// Android (for complicated reasons) misses some standard string conversion functions

namespace std {

inline long stol(const std::string &x, std::size_t* pos = 0, int base = 10) {
	if(!pos)
		return strtol(x.c_str(), nullptr, base);
	char *ptr;
	long rc = strtol(x.c_str(), &ptr, base);
	*pos = ptr - x.c_str();
	return rc;
}

inline long long stoll(const std::string &x, std::size_t* pos = 0, int base = 10) {
	if(!pos)
		return strtoll(x.c_str(), nullptr, base);
	char *ptr;
	long long rc = strtoll(x.c_str(), &ptr, base);
	*pos = ptr - x.c_str();
	return rc;
}

inline double stod(const std::string &x, std::size_t* pos = 0) {
	if(!pos)
		return strtod(x.c_str(), nullptr);
	char *ptr;
	long long rc = strtod(x.c_str(), &ptr);
	*pos = ptr - x.c_str();
	return rc;
}

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

}
#endif

#if __cplusplus <= 201200L

namespace std {

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

#ifndef CUSTOM_INDEX_SEQUENCE
#define CUSTOM_INDEX_SEQUENCE

// index_sequence

template <std::size_t...> struct index_sequence {};

template <std::size_t N, std::size_t... Is>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...> {};

template <std::size_t... Is>
struct make_index_sequence<0u, Is...> : index_sequence<Is...> { using type = index_sequence<Is...>; };

#endif

}

#endif


namespace utils {

const char path_separator = '/';

std::string utf8_encode(const std::string &s);
std::string utf8_encode(const std::wstring &s);
std::wstring utf8_decode(const std::string &s);
int utf8_decode(const std::string &utf8, uint32_t *target);

template <typename T, size_t... Is>
auto gen_tuple_impl(const std::vector<T> &v, std::index_sequence<Is...>)
-> decltype(std::make_tuple(v[Is]...))
{
    return std::make_tuple(v[Is]...);
}

template <size_t N, typename T>
auto gen_tuple(const std::vector<T> &v)
-> decltype(gen_tuple_impl(v, std::make_index_sequence<N>{} ))
{
    return gen_tuple_impl(v, std::make_index_sequence<N>{} );
}

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
		if(crlf && newpos > 0 && s[newpos-1] == 0x0d)
			args.push_back(s.substr(pos, newpos-pos-1));
		else
			args.push_back(s.substr(pos, newpos-pos));
		pos = newpos + l;
	}

	return args;
}
template <typename T>
std::vector<T> split(const T &s, const char *delim, int limit = 0) {
	return split(s, std::string(delim), limit);
}

template <size_t N>
auto splitn(const std::string &text, const std::string &sep)
-> decltype(gen_tuple<N>(std::vector<std::string>()))
{
	return gen_tuple<N>(split(text, sep));
}


//template <typename ... ARGS> std::tuple<ARGS ...> split(const std::string &text,


template<template <typename, typename> class Container, class V, class A>
V join(const Container<V, A> &strings, const V &separator) {
	V out;
	bool first = true;
	for(const auto &s : strings) {
		out += (first ? s : separator + s);
		first = false;
	}
	return out;
}

template<template <typename, typename> class Container, class V, class A>
V join(const Container<V, A> &strings, const wchar_t *separator) {
	return join(strings, V(separator));
}


template<template <typename, typename> class Container, class V, class A>
V join(const Container<V, A> &strings, const char *separator) {
	return join(strings, V(separator));
}

template <typename T> std::string my_tos(const T &t) {
	return std::to_string(t);
}

inline std::string my_tos(const std::string &t) {
	return t;
}

template <class ... ARGS>
std::string join(const std::string &sep, const ARGS& ... args) {
	std::vector<std::string> v { my_tos(args)... };
	return join(v, sep);
}

void replace_char(std::string &s, char c, char r);
void replace_char(char *s, char c, char r);

std::string urlencode(const std::string &s, const std::string &chars);
std::string urldecode(const std::string &s, const std::string &chars = "");

std::string htmldecode(const std::string &source, bool stripTags = false);
std::wstring jis2unicode(uint8_t *text);

void sleepms(unsigned ms);
uint64_t getms();
void makedir(const std::string &name);
void makedirs(const std::string &name);

bool endsWith(const std::string &name, const std::string &ext);
bool startsWith(const std::string &name, const std::string &pref);
//bool contains(const std::string &haystack, const std::string &needle);
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

int shellExec(const std::string &cmd, const std::string &binDir = "");

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

// Can hold smart_ptr or raw pointer
template <typename T> struct Pointer {
	Pointer(std::shared_ptr<T> p) : sptr(p), ptr(p.get()) {}
	Pointer(T *p) : ptr(p) {}
	T* operator ->() const { return ptr; }
	T* get() { return ptr; }
private:
	std::shared_ptr<T> sptr;
	T *ptr = nullptr;
};

// Wrap an object with a name to use where sorting is needed etc
template <typename T> struct Named {
	Named() {}
	Named(const std::string &name, const T &obj) : name(name), obj(obj) {}
	
	operator T&() { return obj; }
	bool operator==(const char *n) const { return strcmp(name.c_str(), n) == 0; }
	bool operator==(const std::string &n) const { return name ==  n; }
	bool operator<(const Named &other) const { return name < other.name; }
private:
	std::string name;
	T obj;
};

};

#endif // UTILS_H
