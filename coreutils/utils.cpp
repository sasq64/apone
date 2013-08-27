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

namespace utils {

using namespace std;

vector<string> split(const string &s, const string &delim) {
	vector<string> args;
	char *temp = new char [ s.length()+1 ];
	char *ptr = temp;
	strcpy(temp, s.c_str());
	while(true) {
		char *arg = strtok(ptr, delim.c_str());			
		if(arg) {
			args.push_back(string(arg));
		} else
			break;
		ptr = nullptr;
	}
	delete [] temp;
	return args;
}

StringTokenizer::StringTokenizer(const string &s, const string &delim) {
	char *temp = new char [ s.length()+1 ];
	char *ptr = temp;
	strcpy(temp, s.c_str());
	int pos = 0;
	while(true) {
		char *arg = strtok(ptr, delim.c_str());			
		if(arg) {
			args.push_back(string(arg));
			delims.push_back(ptr ? 0 : s[pos-1]);//ptr[-1]);
			pos += (strlen(arg)+1);
		} else
			break;
		ptr = nullptr;
	}
	delete [] temp;
}


string urlencode(const string &s, const string &chars) {
	char *target = new char [s.length() * 3 + 1];
	char *ptr = target;
	for(uint i=0; i<s.length(); i++) {
		char c = s[i];
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
	char *target = new char [s.length() + 1];
	char *ptr = target;
	for(uint i=0; i<s.length(); i++) {
		char c = s[i];
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

void makedir(const std::string &name) {

#ifdef WIN32
	mkdir(name.c_str());
#else
	mkdir(name.c_str(), 07777);
#endif
}

void makedirs(const std::string &path) {
	int start = 0;
	while(true) {
		size_t pos = path.find("/", start);
		if(pos != string::npos) {
			makedir(path.substr(0, pos));
			start = pos+1;
		} else
			break;
	}
}

bool endsWith(const string &name, const string &ext) {
	size_t pos = name.rfind(ext);
	return (pos == name.length() - ext.length());
}

void makeLower(string &s) {
	for(uint i=0; i<s.length(); i++)
		s[i] = tolower(s[i]);
}

string path_basename(const string &name) {
	size_t slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	size_t dotPos = name.rfind('.');
	//LOGD("%s : %d %d", name, slashPos, dotPos);
	if(dotPos == string::npos || dotPos < slashPos)
		return name.substr(slashPos);
	return name.substr(slashPos, dotPos-slashPos);
}

string path_directory(const string &name) {
	size_t slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	return name.substr(0, slashPos);
}

string path_filename(const string &name) {
	size_t slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	return name.substr(slashPos);
}

string path_extention(const string &name) {
	size_t dotPos = name.rfind('.');
	size_t slashPos = name.rfind(path_separator);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	if(dotPos == string::npos || dotPos < slashPos)
		return "";
	return name.substr(dotPos+1);

}

string path_suffix(const string &name) { return path_extention(name); }

string path_prefix(const string &name) {
	size_t slashPos = name.rfind(path_separator);
	size_t dotPos = name.find('.', slashPos);
	if(slashPos == string::npos)
		slashPos = 0;
	else
		slashPos++;
	if(dotPos == string::npos || dotPos < slashPos)
		return "";
	return name.substr(slashPos, dotPos-slashPos);
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


}


#ifdef UNIT_TEST

#include "catch.hpp"

TEST_CASE("utils::File", "File operations") {

	using namespace utils;
	using namespace std;

	// Delete to be safe
	std::remove("temp.text");

	// File
	File file { "temp.text" };

	REQUIRE(file.getName() == "temp.text");

	file.write("First line\nSecond line");
	file.close();
	REQUIRE(file.exists());
	REQUIRE(file.getSize() > 5);
	REQUIRE(file.getSize() < 50);

	file = File { "temp.text" };

	file.readAll();
	REQUIRE(file.getPtr() != 0);

	vector<string> lines = file.getLines();

	REQUIRE(lines.size() == 2);

	file.remove();

	REQUIRE(!file.exists());

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
	REQUIRE(path_extention(test1) == "ext");

	REQUIRE(path_extention(test2) == "gz");
	REQUIRE(path_basename(test2) == "file.ext");

	REQUIRE(path_directory(test2) == "");
	REQUIRE(path_filename(test3) == "");
	REQUIRE(path_extention(test3) == "");
	REQUIRE(path_basename(test3) == "");
	REQUIRE(path_directory(test3) == "/my/pa.th");
}

#endif
