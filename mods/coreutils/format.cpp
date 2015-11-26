#include <sys/stat.h>

#include "format.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace utils {

using namespace std;

// FORMAT

void format_stream(std::stringstream &ss, std::string &fmt, const char arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		if(letter == 'd' || letter == 'x')
			ss << (int)(arg&0xff);
		else {
			ss << arg;
		}
	}
}

void format_stream(std::stringstream &ss, std::string &fmt, const unsigned char arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		if(letter == 'd' || letter == 'x')
			ss << (int)(arg&0xff);
		else {
			ss << arg;
		}
	}
}

void format_stream(std::stringstream &ss, std::string &fmt, const signed char arg) {
	char letter;
	if((letter = parse_format(ss, fmt))) {
		if(letter == 'd' || letter == 'x')
			ss << (int)(arg&0xff);
		else {
			ss << arg;
		}
		}
}

void format_stream(stringstream &ss, string &fmt, const vector<int8_t> &bytes) {
	if(parse_format(ss, fmt)) {
		bool first = true;
		int w = ss.width();
		for(auto b : bytes) {
			if(!first) ss << " ";
			ss.width(w);
			ss << (b & 0xff);
			first = false;
		}
	}
}

void format_stream(stringstream &ss, string &fmt, const vector<uint8_t> &bytes) {
	if(parse_format(ss, fmt)) {
		bool first = true;
		int w = ss.width();
		for(auto b : bytes) {
			if(!first) ss << " ";
			ss.width(w);
			ss << (b & 0xff);
			first = false;
		}
	}
}

void format_stream(std::stringstream &ss, std::string &fmt) {
	printf("Why are we here '%s'\n", fmt.c_str());
	ss << fmt;
}

char parse_format(stringstream &ss, string &fmt) {

	size_t pos = 0;

	// Find next format string while replacing %% with %
	while(true) {
		pos = fmt.find_first_of('%', pos);
		if(pos != string::npos && pos < fmt.length()-1) {
			if(fmt[pos+1] == '%') {
				fmt.replace(pos, 2, "%");
				pos++;
			} else
				break;
		} else
			return 0;
	}

	// Put everything before the format string on the stream
	ss << fmt.substr(0, pos);

	char *end = &fmt[fmt.length()];
	char *ptr = &fmt[pos+1];

	if(ptr >= end)
		return 0;
	ss.fill(' ');
	switch(*ptr++) {
	case '0':
		ss.fill('0');
		break;
	case ' ':
		ss.fill(' ');
		break;
	case ',':
		ss.fill(',');
		break;
	case '>':
		ss.fill('\t');
		break;
	case '-':
		ss << left;
		break;
	default:
		ptr--;
		break;
	}

	if(ptr >= end)
		return 0;

	char *endPtr;
	int width = strtol(ptr, &endPtr, 10);

	if(endPtr != nullptr && endPtr > ptr) {
		ss.width(width);
		ptr = endPtr;
	}

	if(ptr >= end)
		return 0;

	char letter = *ptr++;
	if(letter == 'x')
		ss << hex;
	else
		ss << dec;

	// Set the format string to the remainder of the string
	fmt = ptr;

	return letter;
}

void print_fmt(const std::string &ss) {
	fputs(ss.c_str(), stdout);
}



string format(const string &fmt) {
	string fcopy = fmt;
	return fcopy;
}


}


#ifdef UNIT_TEST

#include "catch.hpp"

TEST_CASE("utils::format", "format operations") {

	using namespace utils;
	using namespace std;

	int a = 20;
	const char *b = "test";
	string c = "case";

	string res = format("%x %03x %s %d %s", a, a, b, a, c);
	REQUIRE(res == "14 014 test 20 case");

	vector<int> v { 128, 129, 130, 131 };
	res = format("%02x", v);
	REQUIRE(res == "80 81 82 83");

	//auto s = make_slice(v, 1, 2);
	//res = format("%02x", s);
	//REQUIRE(res == "81 82");
}

#endif
