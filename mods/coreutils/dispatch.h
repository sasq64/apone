#ifndef COREUTILS_DISPATCH_H
#define COREUTILS_DISPATCH_H

#include "utils.h"
#include <functional>

// Type erased base class
template <typename CALLINFO> struct FunctionCaller {
	virtual int call(CALLINFO &ci) = 0;
};

template<typename... X> struct FunctionCallerFunctor;

// Deal with Functors/Lambdas
template <class STATE, class CALLINFO, class FX, class RET, class... ARGS> struct FunctionCallerFunctor<STATE, CALLINFO, FX, RET (FX::*)(ARGS...) const> : public FunctionCaller<CALLINFO>  {

	FunctionCallerFunctor(STATE *state, FX f) : state(state), func(f) {}

	template <size_t ... A> RET apply(CALLINFO &ci, std::index_sequence<A...>) const {
		return func(ci.getArg(A, (ARGS*)nullptr)...);
	}

	int call(CALLINFO &ci) override {
		apply(ci, std::make_index_sequence<sizeof...(ARGS)>());
		return 0;
	}

	FX func;
	STATE *state;
};

// Deal with member functions
template <class CLASS, class CALLINFO, class RET, class... ARGS> struct FunctionCallerMember : public FunctionCaller<CALLINFO> {

	FunctionCallerMember(CLASS *c, RET (CLASS::*f)(ARGS...)) : c(c), func(f) {}

	template <size_t ... A> RET apply(CALLINFO &ci, std::index_sequence<A...>) const {
		return (c->*func)(ci.getArg(A, (ARGS*)nullptr)...);
	}

	int call(CALLINFO &ci) override {
		ci.setReturn(apply(ci, std::make_index_sequence<sizeof...(ARGS)>()));
		return 0;
	}

	RET (CLASS::*func)(ARGS...);
	CLASS *c;

};

// Specialization with void return value
template <class CLASS, class CALLINFO, class... ARGS> struct FunctionCallerMember<CLASS, CALLINFO, void, ARGS...> : public FunctionCaller<CALLINFO> {

	FunctionCallerMember(CLASS *c, void (CLASS::*f)(ARGS...)) : c(c), func(f) {
	}

	template <size_t ... A> void apply(CALLINFO &ci, std::index_sequence<A...>) const {
		(c->*func)(ci.getArg(A, (ARGS*)nullptr)...);
	}

	int call(CALLINFO &ci) override {
		apply(ci, std::make_index_sequence<sizeof...(ARGS)>());
		return 0;
	}

	void (CLASS::*func)(ARGS...);
	CLASS *c;

};

template <class CALLINFO, class FX> FunctionCaller<CALLINFO>* createFunction(FX f) {
	return new FunctionCallerFunctor<void, CALLINFO, FX, decltype(&FX::operator()) >(nullptr, f);
}

template <class CALLINFO, class STATE, class FX> FunctionCaller<CALLINFO>* createFunction(STATE *state, FX f) {
	return new FunctionCallerFunctor<STATE, CALLINFO, FX, decltype(&FX::operator()) >(state, f);
}

template <class CALLINFO, class CLASS, class RET, class... ARGS> FunctionCaller<CALLINFO>* createFunction(CLASS *c, RET (CLASS::*f)(ARGS...)) {
	return new FunctionCallerMember<CLASS, CALLINFO, RET, ARGS...>(c, f);
}

#endif // COREUTILS_DISPATCH_H
