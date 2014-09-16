#ifndef TWEEN_H
#define TWEEN_H

#include <coreutils/log.h>
#include <coreutils/callback.h>

#include <cmath>
#include <functional>
#include <vector>
#include <limits>
#include <memory>
#include <mutex>

namespace tween {

struct BaseRef {};
template <typename OBJ> struct Ref : public BaseRef {
	Ref(std::shared_ptr<OBJ> o) : object(o) {}
	std::shared_ptr<OBJ> object;
};

class Tween;

struct TweenAttrBase {

	TweenAttrBase() {}

	virtual void set(double v, Tween& t) = 0;
	virtual bool compare(void *p) { return false; }

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

	TweenImpl(double t, std::function<double(double)> f) : startTime(t), tween_func(f) {}

	double startTime;
	double totalTime;
	double dspeed;

	bool do_rep = false;
	bool backto = false;

	std::vector<std::shared_ptr<TweenAttrBase>> args;
	std::function<double(double)> tween_func;
	utils::CallbackCaller<Tween&, double> on_complete_cb;
};

class Tween {
protected:
	Tween(std::shared_ptr<TweenImpl> i) : impl(i) {}
public:
	Tween(int dummy = 0) : impl(std::make_shared<TweenImpl>(currentTime, smoothStep_fn)) {}
	//Tween& operator=(const Tween &other) = delete;
	//Tween(const Tween& other) = delete;

	static Tween make();

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

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	Tween& to(T &target, T value, int cycles = 1) {
		for(int i=0; i<target.size(); i++) {
			to(target[i], value[i], cycles);
		}
		return *this;
	}

	template <typename T, typename U> TweenT<T> to(T &target, U value, int cycles = 1) {
		auto *targetp = &target;
		for(auto &t : allTweens) {
			auto it = t->impl->args.begin();
			while(it != t->impl->args.end()) {
				if((*it)->compare((void*)targetp)) {
					// Already tweening
					it = t->impl->args.erase(it);
				} else
					it++;
			}
		}

		auto taptr = std::make_shared<TweenAttr<T>>(targetp, static_cast<T>(value), cycles);
		impl->args.emplace_back(taptr);
		return TweenT<T>(impl, taptr);
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	Tween& from(T &target, T value) {
		for(int i=0; i<target.size(); i++) {
			from(target[i], value[i]);
		}
		return *this;
	}

	template <typename T, typename U> TweenT<T> from(T &target, U value) {
		T realValue = target;
		target = value;
		return to(target, realValue);
	}

	template <typename OBJ> void retain(std::shared_ptr<OBJ> obj) {
		refs.push_back(std::make_shared<Ref<OBJ>>(obj));
	}

	template <typename T, typename U> Tween& fromTo(T &target, U v0, U v1, int cycles = 1) {
		*target = static_cast<T>(v0);
		return to(target, v1);
	}

	template <typename T, typename U> Tween& fromTo(T v0, U v1, int cycles = 1) {
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

	static void updateTweens(double t);

protected:

	bool step();

	static double linear_fn(double t);
	static double smoothStep_fn(double t);
	static double easeInSine_fn (double t);
	static double easeOutSine_fn(double t);
	static double easeInOutSine_fn(double t);
	static double easeInBack_fn (double t);
	static double easeOutBack_fn(double t);
	static double easeInOutBack_fn(double t);
	static double sine_fn(double t);

	std::shared_ptr<TweenImpl> impl;

	static double currentTime;

	std::vector<std::shared_ptr<BaseRef>> refs;

public:
	static std::vector<std::shared_ptr<Tween>> allTweens;
	static std::mutex tweenMutex;
};

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

}; // namespace tween

#endif // TWEEN_H