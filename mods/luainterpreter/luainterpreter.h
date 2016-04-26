#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>
#include <utility>

#include <coreutils/utils.h>


extern "C" {
struct lua_State;
struct luaL_Reg;
};
void _LUA_createtable(struct lua_State *L, int narr, int nrec);
void _LUA_settable(struct lua_State *L, int offs);

class lua_exception : public std::exception {
public:
	lua_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

// Get a value from the lua stack
template <class T> T getArg(struct lua_State *L, int index);
template <> int getArg(struct lua_State *L, int index);
template <> uint32_t getArg(struct lua_State *L, int index);
template <> float getArg(struct lua_State *L, int index);
template <> std::string getArg(struct lua_State *L, int index);
template <> std::vector<std::string> getArg(struct lua_State *L, int index);
template <> std::unordered_map<std::string, std::string> getArg(struct lua_State *L, int index);
template <> void getArg(struct lua_State *L, int index);

// 0-based getArg() -- for use with index_sequence unpacking
template <class T> T getArg0(struct lua_State *L, int index) {
	return getArg<T>(L, index+1);
}

// Push a value to the lua stack
int pushArg(struct lua_State *L, const bool &b);
int pushArg(struct lua_State *L, const int &r);
int pushArg(struct lua_State *L, const unsigned int &r);
int pushArg(struct lua_State *L, const double& a);
int pushArg(struct lua_State *L, const std::string& a);
//int pushArg(struct lua_State *L, const std::vector<std::string>& a);

template <typename T> int pushArg(struct lua_State *L, const std::vector<T>& vec) {
	_LUA_createtable(L, vec.size(), 0);
	int i = 1;
	for(const auto &v : vec) {
		int offs = pushArg(L, i++);
		offs += pushArg(L, v);
		_LUA_settable(L, -1 - offs);
	}
	return 1;
}

template <typename T, typename S> int pushArg(struct lua_State *L, const std::unordered_map<S, T>& amap) {
	_LUA_createtable(L, 0, 0);
	for(const auto &v : amap) {
		int offs = pushArg(L, v.first);
		offs += pushArg(L, v.second);
		_LUA_settable(L, -1 - offs);
	}
	return 1;
}

template <class F, class... A> int pushArg(struct lua_State *L, const F& first, const A& ... tail) {
	return pushArg(L, first) + pushArg(L, tail...);
}


struct FunctionCaller {
	virtual ~FunctionCaller() {};
	virtual int call() = 0;
};

template<typename... X> struct FunctionCallerImpl;


template <class FX, class R, class... ARGS> struct FunctionCallerImpl<FX, R (FX::*)(ARGS...) const> : public FunctionCaller {

	virtual ~FunctionCallerImpl() {
	};

	FunctionCallerImpl(struct lua_State *L, FX f) : L(L), func(f) {
	}

	template <size_t ... A> int apply(std::index_sequence<A...>) {
		return pushArg(L, func(getArg0<ARGS>(L, A)...));
	}

	int call() override {
		return apply(std::make_index_sequence<sizeof...(ARGS)>());
	}

	struct lua_State *L;
	FX func;
};

template <class FX, class... ARGS> struct FunctionCallerImpl<FX,void (FX::*)(ARGS...) const> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, FX f) : L(L), func(f) {
	}

	template <size_t ... A> void apply(std::index_sequence<A...>) {
		func(getArg0<ARGS>(L, A)...);
	}

	int call() override {
		apply(std::make_index_sequence<sizeof...(ARGS)>());
		return 0;
	}

	struct lua_State *L;
	FX func;
};

template <class FX, class R> struct FunctionCallerImpl<FX,R (FX::*)() const> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, FX f) : L(L), func(f) {
	}

	int call() override {
		return pushArg(L, func());
	}

	struct lua_State *L;
	FX func;
};

template <class FX> struct FunctionCallerImpl<FX, void (FX::*)() const> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, FX f) : L(L), func(f) {
	}

	int call() override {
		func();
		return 0;
	}

	struct lua_State *L;
	FX func;
};


class LuaInterpreter {
public:
	LuaInterpreter();
	~LuaInterpreter();

	bool load(const std::string &code, const std::string &name = "");
	bool loadFile(const std::string &name);

	void getGlobalToStack(const std::string &g);
	void setGlobalFromStack(const std::string &g);
	void luaCall(int nargs, int nret);
	void setOuputFunction(std::function<void(const std::string &)> f) {
		outputFunction = f;
	}

	static int proxy_func(lua_State *L);

	void createLuaClosure(const std::string &name, FunctionCaller *fc);

	template <class FX> void registerFunction(const std::string &name, FX f) {
		createLuaClosure(name, new FunctionCallerImpl<FX,decltype(&FX::operator()) >(L, f));
	}

	struct LuaRef {
		LuaRef(const std::string &name, LuaInterpreter& lua) : name(name), lua(lua) {}
		LuaInterpreter &lua;
		std::string name;
		template <typename FX> LuaRef& operator=(FX f) {
			lua.createLuaClosure(name, new FunctionCallerImpl<FX,decltype(&FX::operator()) >(lua.L, f));
			return *this;
		}
	};

	LuaRef operator[](const std::string &name) {
		return LuaRef(name, *this);
	}

	template <class R, class... A> R call(const std::string &f, const A& ... args) {
		getGlobalToStack(f);
		pushArg(L, args...);
		luaCall(sizeof...(args), 1);

		return getArg<R>(L, -1);
	}

	template <class T> void setGlobal(const std::string &name, T arg) {
		pushArg(L, arg);
		setGlobalFromStack(name);
	}

	template <class T> T getGlobal(const std::string &name) {
		getGlobalToStack(name);
		return getArg<T>(L, 1);
	}

private:

	std::unordered_map<std::string, FunctionCaller *> functions;

	static int l_my_print(lua_State* L);
	static const struct luaL_Reg printlib[];

	std::function<void(const std::string &)> outputFunction;
	lua_State *L;
};

#endif // LUAINTERPRETER_H
