#ifndef LUAINTERPRETER_H
#define LUAINTERPRETER_H

#include <string>
#include <functional>
#include <vector>
#include <memory>

struct lua_State;

template <class R> R popArg(struct lua_State *) {
}

template <> double popArg(struct lua_State *);
template <> int popArg(struct lua_State *);

template <class T> T getArg(struct lua_State *L, int index) {
}

template <> int getArg(struct lua_State *L, int index);
template <> std::string getArg(struct lua_State *L, int index);


struct FunctionCaller {
	virtual int call() = 0;
};


int pushArg(struct lua_State *L, int r);

template <class R, class... ARGS> struct FunctionCallerImpl : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, std::function<R(const ARGS& ... )> f) : L(L), func(f) {
	}

	template <class A> int apply() {
		return pushArg(L, func(getArg<A>(L, 1)));
	}

	template <class A, class B> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2)));
	}

	template <class A, class B, class C> int apply() {
		return pushArg(L, func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<B>(L, 2)));
	}

	int call() override {
		return apply<ARGS...>();
	}

	struct lua_State *L;
	std::function<R(const ARGS& ... )> func;
};

template <class... ARGS> struct FunctionCallerImpl<void, ARGS...> : public FunctionCaller {
	FunctionCallerImpl(struct lua_State *L, std::function<void(const ARGS& ... )> f) : L(L), func(f) {
	}

	template <class A> void apply() {
		func(getArg<A>(L, 1));
	}

	template <class A, class B> void apply() {
		func(getArg<A>(L, 1), getArg<B>(L, 2));
	}

	template <class A, class B, class C> void apply() {
		func(getArg<A>(L, 1), getArg<B>(L, 2), getArg<B>(L, 3));
	}

	int call() override {
		apply<ARGS...>();
		return 0;
	}

	struct lua_State *L;
	std::function<void(const ARGS& ... )> func;
};


class LuaInterpreter {
public:
	LuaInterpreter();
	~LuaInterpreter();

	void load(const std::string &code);

	void pushArg(const int& a);
	void pushArg(const double& a);
	void pushArg(const std::string& a);

	void getGlobal(const std::string &g);
	void luaCall(int nargs, int nret);

	std::vector<std::shared_ptr<FunctionCaller>> functions;


	static int proxy_func(lua_State *L);

	void createLuaClosure(const std::string &name, FunctionCaller *fc);

	// template <class R, class... A> void registerFunction(const std::string &name, std::function<R(A ... )> f) {
	// 	FunctionCaller *fc = new FunctionCallerImpl<R, A...>(L, f);
	// 	createLuaClosure(name, fc);
	//}

	template <class R, class A, class B> void registerFunction(const std::string &name, std::function<R(A, B)> f) {
		FunctionCaller *fc = new FunctionCallerImpl<R, A, B>(L, f);
		createLuaClosure(name, fc);
	}

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
