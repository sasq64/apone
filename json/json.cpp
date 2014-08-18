#include "json.h"
#include <cstdio>


//void json_parse(std::string &s) {
//	json_object *jobj = json_tokener_parse(s.c_str());
//
//}


#ifdef UNIT_TEST

#include <catch.hpp>

TEST_CASE("utils::json", "json") {

	using namespace std;

	JSon json("[0,1,2,3]");

	std::vector<int64_t> v = json;
	for(auto &i : v) {
		printf("%d\n", i);
	}

	JSon json2("['aa', 'bb', 'cc']");

	std::vector<string> s = json2;
	for(auto &i : s) {
		printf("%s\n", i.c_str());
	}

	int64_t i = json(1);
	REQUIRE(i == 1);

}

#endif