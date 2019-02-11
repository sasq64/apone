#ifndef TWEEN_H
#define TWEEN_H

#include <coreutils/callback.h>

#include <cmath>
#include <functional>
#include <vector>
#include <limits>
#include <memory>
//#include <coreutils/thread.h>
#include <mutex>

namespace tween {

class Tween;

template <typename C, typename R> using is_compound = typename std::enable_if<std::is_compound<C>::value,R>::type;

struct TweenAttrBase {
    virtual ~TweenAttrBase() {}
	virtual void set(double v, Tween& t) = 0;
	virtual bool compare(void * /*p*/) { return false; }
	double delay = 0.0;
};

template <typename T> struct TweenAttr : public TweenAttrBase {

	TweenAttr(T *target, T value, int cycles) : startValue(*target), delta(value - *target), maxValue(std::numeric_limits<T>::max()), target(target) {
		if(cycles != 1) {
			maxValue = delta+1;
			delta = (delta+1)*cycles-1;
		}
	}

	T startValue;
	T delta;
	T maxValue;

	T *target;

	utils::CallbackCaller<T&, Tween, double> on_update_cb;

	virtual void set(double v, Tween &t) override {
		T newValue = startValue + static_cast<T>(fmod(v * delta, maxValue));
		if(newValue != *target) {
			*target = newValue;
			on_update_cb.call(*target, t, v);
		}
	}

	virtual bool compare(void *p) override {
		return p == static_cast<void*>(target);
	}
};

template <typename T> class TweenT;

struct TweenImpl {

	TweenImpl(double t, bool it = true) : startTime(t), isTweening(it) {}

	~TweenImpl();

	double startTime;
	double totalTime;
	double dspeed;

	bool do_rep = false;
	bool backto = false;

	std::vector<std::shared_ptr<TweenAttrBase>> args;
	std::function<double(double)> tween_func;
	utils::CallbackCaller<Tween&, double> on_complete_cb;
	std::vector<std::shared_ptr<void>> refs;

	bool isTweening = true;

	bool step();

};

class Tween {
protected:
	Tween(std::shared_ptr<TweenImpl> i);
public:
	Tween(int dummy = 0);
	//Tween& operator=(const Tween &other) = delete;
	//Tween(const Tween& other) = delete;

	// Create a new empty Tween. It will start automatically when destroyed, if it was not started manually.
	// This allows you do chain the call; `make().to(...).delay()` etc.
	static Tween make();

	// Manyally start the tween. Will have no effect if already started.
	void start();

	Tween& seconds(float s);
	Tween& speed(float s);
	Tween& linear();
	Tween& smoothstep();
	Tween& easeinback();
	Tween& easeoutback();
	Tween& easeinsine();
	Tween& easeoutsine();
	Tween& sine();
	Tween& repeating();

	//template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	// template <typename T> is_compound<T, Tween&> to(T &target, T value, int cycles = 1) {
	// 	for(int i=0; i<target.size(); i++) {
	// 		to(target[i], value[i], cycles);
	// 	}
	// 	return *this;
	// }

	template <typename T> is_compound<T, Tween&> to(T &target, T value, int cycles = 1) {
		auto it0 = std::begin(target);
		auto it1 = std::begin(value);
		while(it0 != std::end(target)) {
			to(*it0, *it1, cycles);
			++it0;
			++it1;
		}
		return *this;
	}

	template <typename T, typename U> TweenT<T> to(T &target, U value, int cycles = 1) {
		std::lock_guard<std::mutex> guard(tweenMutex);
		auto *targetp = &target;
		for(auto &t : allTweens) {
			auto it = t->args.begin();
			while(it != t->args.end()) {
				if((*it)->compare((void*)targetp)) {
					// Already tweening this value
					it = t->args.erase(it);
				} else
					it++;
			}
		}

		auto taptr = std::make_shared<TweenAttr<T>>(targetp, static_cast<T>(value), cycles);
		impl->args.emplace_back(taptr);
		return TweenT<T>(impl, taptr);
	}

	//template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	template <typename T> is_compound<T, Tween&> from(T &target, T value) {
		auto it0 = std::begin(target);
		auto it1 = std::begin(value);
		while(it0 != std::end(target)) {
			from(*it0, *it1);
			++it0;
			++it1;
		}
		return *this;
	}

	template <typename T, typename U> TweenT<T> from(T &target, U value) {
		T realValue = target;
		target = value;
		return to(target, realValue);
	}

	template <typename OBJ> void retain(std::shared_ptr<OBJ> obj) {
		impl->refs.push_back(std::shared_ptr<void>(obj));
	}

	template <typename T, typename U> TweenT<T> fromTo(T &target, U v0, U v1, int cycles = 1) {
		*target = static_cast<T>(v0);
		return to(target, v1);
	}

	template <typename T, typename U> TweenT<T> fromTo(T v0, U v1, int cycles = 1) {
		auto p = std::make_shared<T>(v0);
		retain(p);
		return to(*(p.get()), v1);
	}

	template <typename F> Tween& onComplete(F&& f) {
		impl->on_complete_cb.callme(std::forward<F>(f));
		return *this;
	}

	void cancel();
	void finish();

	static int updateTweens(double t);
	static void addTween(const TweenImpl& ti);

protected:

	static double default_fn(double t);
	static double linear_fn(double t);
	static double smoothStep_fn(double t);
	static double easeInSine_fn(double t);
	static double easeOutSine_fn(double t);
	static double easeInOutSine_fn(double t);
	static double easeInBack_fn(double t);
	static double easeOutBack_fn(double t);
	static double easeInOutBack_fn(double t);
	static double sine_fn(double t);

	std::shared_ptr<TweenImpl> impl;

public:
	static double currentTime;
	static std::vector<std::shared_ptr<TweenImpl>> allTweens;
	static std::mutex tweenMutex;
};

// A TweenT is a Tween that currently references a tweenable value. You can for instance only call
// onUpdate() on this kind of Tween, since it can refer to the last value added to the Tween
// implementation.
template <typename T> class TweenT : public Tween {
public:

	TweenT(std::shared_ptr<TweenImpl> i, std::shared_ptr<TweenAttr<T>> ta) : Tween(i), ta(ta) {}

	TweenT(const TweenT &t) {
		impl = t.impl;
	}

	template <typename F> TweenT& onUpdate(F &&f) {
		ta->on_update_cb.callme(std::forward<F>(f));
		return *this;
	}

	TweenT& delay(float d) {
		ta->delay = d;
		return *this;
	}

	std::shared_ptr<TweenAttr<T>> ta;
};

} // namespace tween

#endif // TWEEN_H
