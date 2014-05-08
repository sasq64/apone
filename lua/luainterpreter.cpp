#include "luainterpreter.h"

//extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
//}

#include <coreutils/log.h>

using namespace std;

LuaInterpreter::LuaInterpreter() {
	L = luaL_newstate();
	luaL_openlibs(L);
}

LuaInterpreter::~LuaInterpreter() {
	lua_close(L);
}

void LuaInterpreter::load(const string &code) {
	luaL_loadbuffer(L, code.c_str(), code.length(), "line");
	lua_call(L,0,0);
}

/*
uint32_t LuaInterpreter::call(const std::string &f, uint32_t v) {
	lua_getglobal(L, f.c_str());
	lua_pushnumber(L, v);
	lua_call(L, 1, 1);
	double res = lua_tonumber(L, 1);
	return (uint32_t)res;
}*/

void LuaInterpreter::pushArg(const int& a) {
	lua_pushnumber(L, a);
}

void LuaInterpreter::pushArg(const double& a) {
	lua_pushnumber(L, a);
}

void LuaInterpreter::getGlobal(const std::string &g) {
	lua_getglobal(L, g.c_str());
}

void LuaInterpreter::luaCall(int nargs, int nret) {
	LOGD("Call %d %d", nargs, nret);
	lua_call(L, nargs, nret);
}


template <> double popArg(struct lua_State *L) {
	return lua_tonumber(L, 1);
}

template <> int popArg(struct lua_State *L) {
	return lua_tonumber(L, 1);
}

#ifdef UNIT_TEST

#include <catch.hpp>

TEST_CASE("utils::luainterpreter", "interpreter") {

	LuaInterpreter lua;

	REQUIRE(4 == 4);

	const string luaCode = R"(
	function test (a, b)
		return a*2+b
	end
	)";

	lua.load(luaCode);
	auto res = lua.call<double>("test", 4, 2);
	REQUIRE(res == 10);

}

#endif