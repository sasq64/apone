#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <string>
struct lua_State;

template <class R> R popArg(struct lua_State *) {
}

template <> double popArg(struct lua_State *);
template <> int popArg(struct lua_State *);

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
