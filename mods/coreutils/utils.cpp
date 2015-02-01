#include <sys/stat.h>

#include "utils.h"
#include "log.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/time.h>
#include <stdlib.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include <mutex>

#ifdef ANDROID
namespace std {
	long stol(const string &x) { return atol(x.c_str()); }
	long long stoll(const string &x) { return atoll(x.c_str()); }
	long long stod(const string &x) { return atof(x.c_str()); }
}
#endif

namespace utils {

using namespace std;

string rstrip(const string &x, char c) {
	auto l = x.length();
	while(l > 1 && x[l-1] == c)
		l--;
	if(l == x.length())
		return x;
	return x.substr(0, l);
}

string lstrip(const string &x, char c) {
	size_t l = 0;
	while(x[l] && x[l] == c)
		l++;
	if(l == 0)
		return x;
	return x.substr(l);
}

string lrstrip(const string &x, char c) {
	size_t l = 0;
	while(x[l] && x[l] == c)
		l++;
	size_t r = l;
	while(x[r] && x[r] != c)
		r++;
	return x.substr(l, r-l);
}

vector<string> text_wrap(const string &t, int width, int initialWidth) {

	vector<string> lines;
	size_t start = 0;
	size_t end = width;
	int subseqWidth = width;
	if(initialWidth >= 0)
		width = initialWidth;

	string text = t;
	for(auto &c : text) {
		if(c == 0xa)
			c = ' ';
	}

	// Find space from right
	LOGD("wrapping '%s' at %d,%d", text, width, subseqWidth);



	while(true) {
		if(end > text.length()) {
			lines.push_back(text.substr(start));
			break;
		}
		auto pos = text.rfind(' ', end);
		if(pos != string::npos && pos > start) {
			LOGD("Breaking at %d,%d", start, pos - start);
			lines.push_back(text.substr(start, pos - start));
			start = pos+1;
		} else {
			LOGD("Found no space, at %d,%d", start,width);
			lines.push_back(text.substr(start, width));
			start += width;
		}
		width = subseqWidth;
		end = start + width;

	}
	return lines;
}

static int16_t decode(const string &symbol) {
	static unordered_map<std::string, int16_t> codes = {
		{ "amp", '&' },
		{ "gt", '>' },
		{ "lt", '<' }
	};

	if(codes.count(symbol))
		return codes[symbol];
	return '?';
}

string htmldecode(const string &s) {

	char target [s.length() + 1];
	char *ptr = target;
	char symbol[32];
	char *sptr;

	for(uint i=0; i<s.length(); i++) {
		uint16_t c = s[i];
		if(c == '&') {
			sptr = symbol;
			int saved = i;
			i++;
			while(isalnum(s[i]))
				*sptr++ = s[i++];
			*sptr = 0;
			if(s[i] == ';') {
				c = decode(symbol);
			} else
				i = saved;
			LOGD("%s -> %02x", symbol, c);

		}
		*ptr ++ = c;
	}
	*ptr = 0;
	return string(target);

}

string urlencode(const string &s, const string &chars) {
	char target [s.length() * 3 + 1];
	char *ptr = target;
	for(uint i=0; i<s.length(); i++) {
		auto c = s[i];
		if(chars.find(c) != string::npos) {
			sprintf(ptr, "%%%02x", c);
			ptr += 3;
		} else
			*ptr++ = c;
	}
	*ptr = 0;
	return string(target);
}

string urldecode(const string &s, const string &chars) {
	char target [s.length() + 1];
	char *ptr = target;
	for(uint i=0; i<s.length(); i++) {
		auto c = s[i];
		if(c == '%') {
			*ptr++ = strtol(s.substr(i+1,2).c_str(), nullptr, 16);
			i += 2;
		} else
			*ptr++ = c;
	}
	*ptr = 0;
	return string(target);
}

void sleepms(uint ms) {
#ifdef WIN32
	Sleep(ms);
#else
	usleep(ms*1000);
#endif
}

uint64_t getms() {
#ifdef EMSCRIPTEN
	return (uint64_t)emscripten_get_now();
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000 + tv.tv_usec) / 1000;
#endif
}


bool isalpha(const string &s) {
	for(const auto &c : s) {
		LOGD("c %c", c);
		if(!::isalpha(c))
			return false;
	}
	return true;
}

float clamp(float x, float a0, float a1) {
	return min(max(x, a0), a1); 
}

void makedir(const std::string &name) {

#ifdef _WIN32
	mkdir(name.c_str());
#else
	mkdir(name.c_str(), 07777);
#endif
}

void makedirs(const std::string &path) {
	int start = 0;
	while(true) {
		auto pos = path.find("/", start);
		if(pos != string::npos) {
			makedir(path.substr(0, pos));
			start = pos+1;
		} else
			break;
	}
}

bool endsWith(const string &name, const string &ext) {
	auto pos = name.rfind(ext);
	return (pos == name.length() - ext.length());
}

bool startsWith(const string &name, const string &pref) {
	auto pos = name.find(pref);
	return (pos == 0);
}

void makeLower(string &s) {
	for(uint i=0; i<s.length(); i++)
		s[i] = tolower(s[i]);
}

string toLower(const string &s) {
	string s2 = s;
	makeLower(s2);
	return s2;
}

string path_basename(const string &name) {
	auto slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	auto dotPos = name.rfind('.');
	//LOGD("%s : %d %d", name, slashPos, dotPos);
	if(dotPos == string::npos || dotPos < slashPos)
		return name.substr(slashPos);
	return name.substr(slashPos, dotPos-slashPos);
}

string path_directory(const string &name) {
	auto slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	return name.substr(0, slashPos);
}

string path_filename(const string &name) {
	auto slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	return name.substr(slashPos);
}

string path_extension(const string &name) {
	auto dotPos = name.rfind('.');
	auto slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	if(dotPos == string::npos || dotPos < slashPos)
		return "";
	return name.substr(dotPos+1);

}

string path_suffix(const string &name) { return path_extension(name); }

string path_prefix(const string &name) {
	auto slashPos = name.rfind(path_separator);
	auto dotPos = name.find('.', slashPos);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	if(dotPos == string::npos || dotPos < slashPos)
		return "";
	return name.substr(slashPos, dotPos-slashPos);
}

static const uint32_t offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const uint8_t trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

wstring utf8_decode(const string &s)
{
    size_t i = 0;
    wstring result;
    while (i < s.length()) {
        auto nb = trailingBytesForUTF8[(uint8_t)s[i]];
        if(s[i] == 0)
        	break;
        wchar_t ch = 0;
        switch (nb) {
            /* these fall through deliberately */
        case 3: ch |= (unsigned char)s[i++]; ch <<= 6;
        case 2: ch |= (unsigned char)s[i++]; ch <<= 6;
        case 1: ch |= (unsigned char)s[i++]; ch <<= 6;
        case 0: ch |= (unsigned char)s[i++];
        }
        ch -= offsetsFromUTF8[nb];
        result.push_back(ch);
    }
    return result;
}

string utf8_encode(const string &s) {
	string out;
	for(const auto &c : s) {
		if(c <= 0x7f)
			out.push_back(c);
		else {
			out.push_back(0xC0 | (c >> 6));
			out.push_back(0x80 | (c & 0x3F));
		}
	}
	return out;
}


string utf8_encode(const wstring &s) {
	string out;
	for(const auto &c : s) {
		if(c <= 0x7f)
			out.push_back(c);
		else if(c < 0x800) {
			out.push_back(0xC0 | (c >> 6));
			out.push_back(0x80 | (c & 0x3F));
		} else /*if (c < 0x10000) */ {
			out.push_back(0xE0 | (c >> 12));
			out.push_back(0x80 | (c >> 6));
			out.push_back(0x80 | (c & 0x3F));
		}
	}
	return out;
}

void replace_char(std::string &s, char c, char r) {
	replace_char(&s[0], c, r);
}

void replace_char(char *s, char c, char r) {
	while(*s) {
		if(*s == c)
			*s = r;
		s++;
	}
}

std::string current_exe_path() {
#ifdef LINUX
	static char buf[1024];
	int rc = readlink("/proc/self/exe", buf, sizeof(buf)-1);
	if(rc >= 0) {
		buf[rc] = 0;
		return path_directory(buf);
	} return "";
#else
	return "";
#endif
}

static bool performCalled = false;
static vector<function<void()>> callbacks;

void schedule_callback(std::function<void()> f) {
	if(performCalled)
		callbacks.push_back(f);
	else
		f();
}

void perform_callbacks() {
	performCalled = true;
	for(const auto &f : callbacks) {
		f();
	}
	callbacks.clear();
}

}

#ifdef UNIT_TEST

#include "catch.hpp"

TEST_CASE("utils::text", "Text operations") {

	using namespace utils;
	using namespace std;

	string text = "This is a journey into sound. Stereophonic sound. Stereophonic sound with mounds of boundless hounds rounding the ground.";

	auto lines = text_wrap(text, 25);
	for(const auto &l : lines) {
		LOGD("Line:%s", l);
	}
}

TEST_CASE("utils::path", "Path name operations") {

	using namespace utils;
	using namespace std;

	const string test1 = "c:/path/to/my/file.ext";
	const string test2 = "file.ext.gz";
	const string test3 = "/my/pa.th/";

	REQUIRE(path_basename(test1) == "file");
	REQUIRE(path_directory(test1) == "c:/path/to/my");
	REQUIRE(path_filename(test1) == "file.ext");
	REQUIRE(path_extension(test1) == "ext");

	REQUIRE(path_extension(test2) == "gz");
	REQUIRE(path_basename(test2) == "file.ext");

	REQUIRE(path_directory(test2) == "");
	REQUIRE(path_filename(test3) == "");
	REQUIRE(path_extension(test3) == "");
	REQUIRE(path_basename(test3) == "");
	REQUIRE(path_directory(test3) == "/my/pa.th");
}

#endif

