
#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#else

#include <coreutils/format.h>
#include <coreutils/log.h>
#include <coreutils/var.h>
#include <coreutils/file.h>

#include <vector>

using namespace std;
using namespace utils;

int main() {

	vector<uint8_t> v { 0x11, 0xff, 0x00, 0x45 };
	string s = "Some text";
	int n = 12345;

	auto fmt = format("%s %d [%02x]", s, n, v);
	LOGD("We got ;%s;", fmt);

	File f { "output.txt" };
	f.write(fmt);
	f.close();

	var x,y,z;

	x = "This is a string";
	y = 12345;
	z = "0x12345";

	int zn = z;
	string ys = y;

	LOGD("We got: %d '%s'", zn, ys);

	return 0;
}


#endif