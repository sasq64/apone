#include "luainterpreter.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <coreutils/log.h>

using namespace std;

void _LUA_createtable(struct lua_State *L, int narr, int nrec) {
	lua_createtable(L, narr, nrec);
}

void _LUA_settable(struct lua_State *L, int offs) {
	lua_settable(L, offs);
}

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

template <> std::vector<std::string> getArg(struct lua_State *L, int index) {

	if(index < 0) index--;
	std::vector<std::string> vec;
	lua_pushnil(L);
	while(lua_next(L, index) != 0) {
		vec.push_back(lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	return vec;
}

template <> std::unordered_map<std::string, std::string> getArg(struct lua_State *L, int index) {

	if(index < 0) index--;
	std::unordered_map<std::string, std::string> smap;
	lua_pushnil(L);
	while(lua_next(L, index) != 0) {
		const char *key = lua_tostring(L, -2);
		const char *value = lua_tostring(L, -1);
		smap[key] = value;
		lua_pop(L, 1);
	}
	return smap;

}


int pushArg(struct lua_State *L, const int& a) {
	lua_pushnumber(L, a);
	return 1;
}

int pushArg(struct lua_State *L, const unsigned int& a) {
	lua_pushnumber(L, a);
	return 1;
}

int pushArg(struct lua_State *L, const double& a) {
	lua_pushnumber(L, a);
	return 1;
}

int pushArg(struct lua_State *L, const std::string& a) {
	lua_pushstring(L, a.c_str());
	return 1;
}

int LuaInterpreter::proxy_func(lua_State *L) {
	FunctionCaller *fc = (FunctionCaller*)(lua_touserdata(L, lua_upvalueindex(1)));
	return fc->call();
}

/*
void create_meta(FunctionCaller *fc
{

		lua_createtable(L, 0, 0);

		lua_pushlightuserdata(L, fc);
	    lua_pushcclosure(L, proxy_func, 1);

		lua_pushstring(L, "__tostring");
		lua_pushcclosure(L, proxy_func, 1);		
		lua_settable(L, -3);
		lua_setmetatable(L, -1);
}
*/

void LuaInterpreter::createLuaClosure(const std::string &name, FunctionCaller *fc) {
	lua_pushlightuserdata(L, fc);
    lua_pushcclosure(L, proxy_func, 1);
    lua_setglobal(L, name.c_str());

    functions[name] = fc;
}


const struct luaL_Reg LuaInterpreter::printlib[] = {
	{ "print", LuaInterpreter::l_my_print },
	{ nullptr, nullptr }
};

int LuaInterpreter::l_my_print(lua_State* L) {

	LuaInterpreter *li = (LuaInterpreter*)lua_touserdata(L, lua_upvalueindex(1));
    int nargs = lua_gettop(L);
    for (int i=1; i <= nargs; ++i) {
    	const char *s = lua_tostring(L, i);
    	if(s) {
			if(li->outputFunction)
				li->outputFunction(s);
			else
				puts(s);
		}
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
	for(auto &p : functions) {
		delete p.second;
	}
}

bool LuaInterpreter::load(const string &code, const string &name) {
	if(luaL_loadbuffer(L, code.c_str(), code.length(), name.c_str()) == LUA_OK) {
		int rc = lua_pcall(L, 0, 0, 0);
		if(rc != LUA_OK) {
			const char *s = lua_tostring(L, -1);
			//LOGD("MSG:%s", s);
			lua_pop(L, 1);
			throw lua_exception(s);
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

void LuaInterpreter::getGlobalToStack(const std::string &g) {
	lua_getglobal(L, g.c_str());
}

void LuaInterpreter::setGlobalFromStack(const string &name) {
	lua_setglobal(L, name.c_str());
}

void LuaInterpreter::luaCall(int nargs, int nret) {
	LOGD("Call %d %d", nargs, nret);
	lua_call(L, nargs, nret);
}

#ifdef UNIT_TEST

#include <catch.hpp>

TEST_CASE("utils::luainterpreter", "interpreter") {

	LuaInterpreter lua;

	lua.registerFunction<int, string, int, float>("testFunc", [=](string s, int x, float f) -> int {
		LOGD("Got '%s' and %d", s, x);
		return 1 + x + f;
	});

	//lua.registerFunction<void, int>("testVFunc", [=](string s, int x) {
	//	LOGD("Got '%s' and %d", s, x);
	//});

	const string luaCode = R"(
	function test (a, b)
		testVFunc('arne', 199)
		x = testFunc('hello', 2, 3)
		return x * 2 + a + b
	end
	)";

	lua.load(luaCode);
	auto res = lua.call<double>("test", 4, 2);
	REQUIRE(res == 18);

}

#endif