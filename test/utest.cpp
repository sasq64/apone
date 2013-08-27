
#ifdef UNIT_TEST
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#else

#include <coreutils/format.h>
#include <coreutils/log.h>

using namespace utils;

int main() {

	auto s = format("%s %d", "hello", 32);
	LOGD("We got ;%s;", s);
	return 0;
}


#endif