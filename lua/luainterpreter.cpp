#include "luainterpreter.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <coreutils/log.h>

using namespace std;

template <> int getArg(struct lua_State *L, int index) {
	return lua_tointeger(L, index);
}

template <> uint32_t getArg(struct lua_State *L, int index) {
	return lua_tointeger(L, index);
}

template <> float getArg(struct lua_State *L, int index) {
	return lua_tonumber(L, index);
}

template <> std::string getArg(struct lua_State *L, int index) {
	return string(lua_tostring(L, index));
}

int pushArg(struct lua_State *L, int r) {
	lua_pushinteger(L, r);
	return 1;
};

int LuaInterpreter::proxy_func(lua_State *L) {
	FunctionCaller *fc = (FunctionCaller*)(lua_touserdata(L, lua_upvalueindex(1)));
	return fc->call();
}

void LuaInterpreter::createLuaClosure(const std::string &name, FunctionCaller *fc) {
	lua_pushlightuserdata(L, fc);
    lua_pushcclosure(L, proxy_func, 1);
    lua_setglobal(L, name.c_str());
}


static const struct luaL_Reg printlib[] = {
	{ "print", LuaInterpreter::l_my_print },
	{ nullptr, nullptr }
};

int LuaInterpreter::l_my_print(lua_State* L) {

	LuaInterpreter *li = (LuaInterpreter*)lua_touserdata(L, lua_upvalueindex(1));
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; ++i) {
		if(li->outputFunction)
			li->outputFunction(lua_tostring(L, i));
		else
			LOGD("LUA:%s", lua_tostring(L, i));
    }
	if(li->outputFunction)
		li->outputFunction("\n");
    return 0;
}

LuaInterpreter::LuaInterpreter() {
	L = luaL_newstate();
	luaL_openlibs(L);
	lua_getglobal(L, "_G");
	lua_pushlightuserdata(L, this);
	luaL_setfuncs(L, printlib, 1);
	lua_pop(L, 1);}

LuaInterpreter::~LuaInterpreter() {
	lua_close(L);
}

bool LuaInterpreter::load(const string &code) {
	if(luaL_loadbuffer(L, code.c_str(), code.length(), "line") == LUA_OK) {
		int rc = lua_pcall(L, 0, 0, 0);
		if(rc != LUA_OK) {
			const char *s = lua_tostring(L, -1);
			LOGD("MSG:%s", s);
			lua_pop(L, 1);
		}
	} else
		return false;
	return true;
}

bool LuaInterpreter::loadFile(const string &fileName) {
	if(luaL_loadfile(L, fileName.c_str()) == LUA_OK) {
		int rc = lua_pcall(L, 0, 0, 0);
		if(rc != LUA_OK) {
			const char *s = lua_tostring(L, -1);
			LOGD("MSG:%s", s);
			lua_pop(L, 1);
		}
	} else
		return false;
	return true;
}

void LuaInterpreter::pushArg(const int& a) {
	lua_pushnumber(L, a);
}

void LuaInterpreter::pushArg(const double& a) {
	lua_pushnumber(L, a);
}

void LuaInterpreter::pushArg(const std::string& a) {
	lua_pushstring(L, a.c_str());
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

	lua.registerFunction<int, string, int, float>("testFunc", [=](string s, int x, float f) -> int {
		LOGD("Got '%s' and %d", s, x);
		return 3;
	});

	//lua.registerFunction<void, int>("testVFunc", [=](string s, int x) {
	//	LOGD("Got '%s' and %d", s, x);
	//});

	const string luaCode = R"(
	function test (a, b)
		testVFunc('arne', 199)
		x = testFunc('hello', 13)
		return a*2+b+x
	end
	)";

	lua.load(luaCode);
	auto res = lua.call<double>("test", 4, 2);
	REQUIRE(res == 13);

}

#endif