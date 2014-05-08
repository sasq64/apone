#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <string>
#include <functional>
#include <vector>

//#include <lua.h>
struct lua_State;

template <class R> R popArg(struct lua_State *) {
}

template <> double popArg(struct lua_State *);
template <> int popArg(struct lua_State *);

/*
template <class... ARGS> struct FunctionCaller {
	FunctionCaller(std::function<void(const ARGS& ... )> f) : func(f) {
	}

	template <class T> void getArg(int index, T &target) {
	}

	int getArg(lua_State *L, int index, int &target);// {
//			target = lua_tointeger(L, index);
//		}

	ARGS getArgs(lua_State *L) {
		a = getArg<A>(L, 0);
		b = getArg<B>(L, 1);
	}

	template <class A, class B> void call() {
		func(getArg<A>(L, 0), getArg<B>(L, 1));
	}


	void call() {
		func(getArgs());
	}

	std::function<void(const ARGS& ... )> func;
};
*/


class LuaInterpreter {
public:
	LuaInterpreter();
	~LuaInterpreter();

	void load(const std::string &code);

	//uint32_t call(const std::string &f, uint32_t v);

	void pushArg(const int& a);
	void pushArg(const double& a);
	void getGlobal(const std::string &g);
	void luaCall(int nargs, int nret);

	std::vector<std::function<void()>> funcs;

/*
	template <class... A> void registerFunction(const std::string &name, std::function<void(const A& ... )> f) {

		functions.push_back([=]() {

			f(
		});

		lua_pushlightuserdata(L, this);
	    lua_pushcclosure(L, proxy_func, 1);
	    lua_setglobal(L, name.c_str());

		std::function<int(lua_State *)> f2;
		lua_register(L, name.c_str(), f2);
	}
*/
	template <class F, class... A> void pushArg(const F& first, const A& ... tail) {
		pushArg(first);
		pushArg(tail...);
	}

	template <class R, class... A> R call(const std::string &f, const A& ... args) {
		getGlobal(f);
		pushArg(args...);
		luaCall(sizeof...(args), 1);
		return popArg<R>(L);
	}

private:
	lua_State *L;
};

#endif // LUAINTERPRETER_H
