#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

struct lua_State;
struct luaL_Reg;
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

// Push a value to the lua stack
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

template <class R, class... ARGS> struct FunctionCallerImpl : public FunctionCaller {

	virtual ~FunctionCallerImpl() {
	};


	FunctionCallerImpl(struct lua_State *L, std::function<R(ARGS ... )> f) : L(L), func(f) {
	}

	template <class A> int apply() {
		return pushArg(L, func(getArg<A>(L, 1)));
	}

	template <class A, class B> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2)));
	}

	template <class A, class B, class C> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<B>(L, 3)));
	}

	template <class A, class B, class C, class D> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<B>(L, 3), getArg<B>(L, 4)));
	}

	template <class A, class B, class C, class D, class E> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<B>(L, 3), getArg<B>(L, 4), getArg<B>(L, 5)));
	}

	template <class A, class B, class C, class D, class E, class F> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<B>(L, 3), getArg<B>(L, 4), getArg<B>(L, 5), getArg<B>(L, 6)));
	}

	int call() override {
		return apply<ARGS...>();
	}

	struct lua_State *L;
	std::function<R(ARGS ... )> func;
};

template <class... ARGS> struct FunctionCallerImpl<void, ARGS...> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, std::function<void(ARGS ... )> f) : L(L), func(f) {
	}

	template <class A> void apply() {
		func(getArg<A>(L, 1));
	}

	template <class A, class B> void apply() {
		func(getArg<A>(L, 1), getArg<B>(L, 2));
	}

	template <class A, class B, class C> void apply() {
		func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<C>(L, 3));
	}

	template <class A, class B, class C, class D> void apply() {
		func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<C>(L, 3), getArg<D>(L, 4));
	}

	template <class A, class B, class C, class D, class E> void apply() {
		func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<C>(L, 3), getArg<D>(L, 4), getArg<E>(L, 5));
	}

	int call() override {
		apply<ARGS...>();
		return 0;
	}

	struct lua_State *L;
	std::function<void(ARGS ... )> func;
};

template <class R> struct FunctionCallerImpl<R> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, std::function<R()> f) : L(L), func(f) {
	}

	int call() override {
		return pushArg(L, func());
	}

	struct lua_State *L;
	std::function<R()> func;
};

template <> struct FunctionCallerImpl<void> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, std::function<void()> f) : L(L), func(f) {
	}

	int call() override {
		func();
		return 0;
	}

	struct lua_State *L;
	std::function<void()> func;
};

class LuaInterpreter {
public:
	LuaInterpreter();
	~LuaInterpreter();

	bool load(const std::string &code, const std::string &name = "");
	bool loadFile(const std::string &name);

	void getGlobal(const std::string &g);
	void setGlobal(const std::string &g);
	void luaCall(int nargs, int nret);
	void setOuputFunction(std::function<void(const std::string &)> f) {
		outputFunction = f;
	}


	static int proxy_func(lua_State *L);

	void createLuaClosure(const std::string &name, FunctionCaller *fc);

	// Sadly this doesn't work snce we want to use lambdas as parameters, and C++ cant resolve a lambda
	// to a template function
	//template <class R, class... A> void registerFunction(const std::string &name, std::function<R(A ... )> f) {
	//	createLuaClosure(name, new FunctionCallerImpl<R, A...>(L, f));
	//}

	template <class R> void registerFunction(const std::string &name, std::function<R()> f) {
		createLuaClosure(name, new FunctionCallerImpl<R>(L, f));
	}

	template <class R, class A> void registerFunction(const std::string &name, std::function<R(A)> f) {
		createLuaClosure(name, new FunctionCallerImpl<R, A>(L, f));
	}

	template <class R, class A, class B> void registerFunction(const std::string &name, std::function<R(A, B)> f) {
		createLuaClosure(name, new FunctionCallerImpl<R, A, B>(L, f));
	}

	template <class R, class A, class B, class C> void registerFunction(const std::string &name, std::function<R(A, B, C)> f) {
		createLuaClosure(name, new FunctionCallerImpl<R, A, B, C>(L, f));
	}

	template <class R, class A, class B, class C, class D> void registerFunction(const std::string &name, std::function<R(A, B, C, D)> f) {
		createLuaClosure(name, new FunctionCallerImpl<R, A, B, C, D>(L, f));
	}

	template <class R, class A, class B, class C, class D, class E> void registerFunction(const std::string &name, std::function<R(A, B, C, D, E)> f) {
		createLuaClosure(name, new FunctionCallerImpl<R, A, B, C, D, E>(L, f));
	}

	template <class R, class A, class B, class C, class D, class E, class F> void registerFunction(const std::string &name, std::function<R(A, B, C, D, E, F)> f) {
		createLuaClosure(name, new FunctionCallerImpl<R, A, B, C, D, E, F>(L, f));
	}


	template <class R, class... A> R call(const std::string &f, const A& ... args) {
		getGlobal(f);
		pushArg(L, args...);
		luaCall(sizeof...(args), 1);
		
		auto x = getArg<R>(L, -1);
		return x;
	}

	template <class T> void setGlobal(const std::string &name, T arg) {
		pushArg(L, arg);
		setGlobal(name);
	}

private:

	std::unordered_map<std::string, FunctionCaller *> functions;

	static int l_my_print(lua_State* L);
	static const struct luaL_Reg printlib[];

	std::function<void(const std::string &)> outputFunction;
	lua_State *L;
};

#endif // LUAINTERPRETER_H
