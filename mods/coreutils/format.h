#ifndef COREUTILS_FORMAT_H
#define COREUTILS_FORMAT_H

#include <stdint.h>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>

namespace utils {

char parse_format(std::stringstream &ss, std::string &fmt);

template <class T> void format_stream(std::stringstream &ss, std::string &fmt, const T *arg) {
	if(parse_format(ss, fmt))
		ss << arg;
}

template<template <typename, typename> class Container, class V, class A> void format_stream(std::stringstream &ss, std::string &fmt, Container<V, A> const& arg) {
	if(parse_format(ss, fmt)) {
		bool first = true;
		int w = (int)ss.width();
		for(auto b : arg) {
			if(!first) ss << ss.fill();
			ss.width(w);
			ss << b;
			first = false;
		}
	}
}

template<template <typename, typename> class Container, class A> void format_stream(std::stringstream &ss, std::string &fmt, Container<char, A> const& arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		bool first = true;
		int w = (int)ss.width();
		for(auto b : arg) {
			if(!first) ss << ss.fill();
			ss.width(w);
			if(letter == 'd' || letter == 'x')
				ss << (int)(b&0xff);
			else {
				ss << b;
			}
			first = false;
		}
	}
}

template<template <typename, typename> class Container, class A> void format_stream(std::stringstream &ss, std::string &fmt, Container<unsigned char, A> const& arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		bool first = true;
		int w = (int)ss.width();
		for(auto b : arg) {
			if(!first) ss << ss.fill();
			ss.width(w);
			if(letter == 'd' || letter == 'x')
				ss << (int)(b&0xff);
			else {
				ss << b;
			}
			first = false;
		}
	}
}

template<template <typename, typename> class Container, class A> void format_stream(std::stringstream &ss, std::string &fmt, Container<signed char, A> const& arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		bool first = true;
		int w = (int)ss.width();
		for(auto b : arg) {
			if(!first) ss << ss.fill();
			ss.width(w);
			if(letter == 'd' || letter == 'x')
				ss << (int)(b&0xff);
			else {
				ss << b;
			}
			first = false;
		}
	}
}

void format_stream(std::stringstream &ss, std::string &fmt);
void format_stream(std::stringstream &ss, std::string &fmt, const char arg);
void format_stream(std::stringstream &ss, std::string &fmt, const unsigned char arg);
void format_stream(std::stringstream &ss, std::string &fmt, const signed char arg);


template <class T> void format_stream(std::stringstream &ss, std::string &fmt, const T& arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		ss << arg;
	}
}

template <class A, class... B>
void format_stream(std::stringstream &ss, std::string &fmt, const A &head, const B& ... tail)
{
	format_stream(ss, fmt, head);
	format_stream(ss, fmt, tail...);
}

std::string format(const std::string &fmt);

template <class... A> std::string format(const std::string &fmt, const A& ... args)
{
	std::string fcopy = fmt;
	std::stringstream ss;
	ss << std::boolalpha;
	format_stream(ss, fcopy, args...);
	ss << fcopy;
	return ss.str();
}

template <class... A> void print_fmt(const std::string &fmt, const A& ... args) {
	std::string fcopy = fmt;
	std::stringstream ss;
	ss << std::boolalpha;
	format_stream(ss, fcopy, args...);
	ss << fcopy;
	fputs(ss.str().c_str(), stdout);
}

void print_fmt(const std::string &fmt);

}

#endif // COREUTILS_FORMAT_H
