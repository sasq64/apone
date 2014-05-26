#ifndef TWEEN_H
#define TWEEN_H

#include <coreutils/log.h>

#include <cmath>
#include <functional>
#include <vector>
#include <limits>
#include <memory>

namespace tween {

struct TweenAttrBase {

	TweenAttrBase(double target, double value) : startValue(target), delta(value - target), maxValue(std::numeric_limits<double>::max()) {}

	virtual void set(double v) = 0;
	virtual bool compare(void *p) { return false; }

	double startValue;
	double delta;
	double delay;
	double maxValue;

	std::function<double(double)> tweenFunc;

};


template <typename T> class TweenAttr : public TweenAttrBase {
public:
	TweenAttr(T &target, T value) : TweenAttrBase(target, value), target(&target) {}
	T *target;

	virtual void set(double v) override {
		*target = static_cast<T>(v);
	}

	virtual bool compare(void *p) override {
		return p == (void*)target;
	}
};

class Tween;

class TweenHolder {
public:
	TweenHolder() {}
	TweenHolder(std::shared_ptr<Tween> t) : tween(t) {}
	void cancel();
	bool done();
	bool valid();
	void finish();
private:
	std::shared_ptr<Tween> tween;
};

class Tween {
public:
	Tween() : delay(0.0), startTime(currentTime), tweenFunc(smoothStep_fn) {}

	Tween& operator=(const Tween &other) = delete;
	Tween(const Tween& other) = delete;

	operator TweenHolder() {
		return TweenHolder(Tween::allTweens.back());
	}

	Tween& seconds(float s) {
		totalTime = s;
		return *this;
	}

	Tween& speed(float s) {
		dspeed = s;
		return *this;
	}

	Tween& linear() {
		tweenFunc = linear_fn;
		return *this;
	}

	Tween& sine() {
		tweenFunc = sine_fn;
		backto = true;
		return *this;
	}

	Tween& repeating() {
		do_rep = true;
		return *this;
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	Tween& to(T &target, T value, int cycles = 1) {
		for(int i=0; i<target.size(); i++) {
			to(target[i], value[i], cycles);
		}
		return *this;
	}

	template <typename T, typename U> Tween& to(T &target, U value, int cycles = 1) {
	
		for(auto &t : allTweens) {
			auto it = t->args.begin();
			while(it != t->args.end()) {
				if(it->get()->compare((void*)&target)) {
					LOGD("ALREADY TWEENING!");
					it = t->args.erase(it);
				} else
					it++;
			}
		}

		args.emplace_back(std::make_shared<TweenAttr<T>>(target, value));
		auto &a = args.back();

		if(cycles != 1) {
			a->maxValue = a->delta+1;
			a->delta = (a->delta+1)*cycles-1;
		}

		a->tweenFunc = tweenFunc;
		a->delay = delay;
		return *this;
	}


	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	Tween& from(T &target, T value) {
		for(int i=0; i<target.size(); i++) {
			from(target[i], value[i]);
		}
		return *this;
	}

	template <typename T, typename U> Tween& from(T &target, U value) {

		to(target, value);
		auto &a = args.back();
		//LOGD("Negating delta %f and startValue %f -> %f", a->delta, a->startValue, a->startValue + a->delta);
		a->startValue += a->delta;
		a->delta = -a->delta;
		a->set(a->startValue);
		return *this;
	}

	Tween& on_complete(std::function<void()> f) {
		onCompleteFunc = f;
		return *this;
	}

	void finish() {
		for(auto &a : args) {
			a->set(a->startValue + fmod(a->delta, a->maxValue));
		}
	}

	bool step() { // t : 0 -> 1
		size_t ended = 0;
		for(auto &a : args) {
			float t = (currentTime - startTime - a->delay) / totalTime;
			if(t < 0.0)
				continue;
			if(t > 1.0) {
				if(do_rep)
					t -= 1.0;
				else if(backto) {
					ended++;
					a->set(a->startValue);
				}else {
					ended++;
					a->set(a->startValue + fmod(a->delta, a->maxValue));
					continue;
				}
			}
			a->set(a->startValue + fmod(a->tweenFunc(t) * a->delta, a->maxValue));
		}
		return ended < args.size();
	};

	static void updateTweens(double t) {
		std::vector<std::function<void()>> callbacks;
		currentTime = t;
		auto it = allTweens.begin();
		//LOGD("We have %d tweens", allTweens.size());
		while(it != allTweens.end()) {
			if(!(*it)->step()) {
				if((*it)->onCompleteFunc)
					callbacks.push_back((*it)->onCompleteFunc);
				it = allTweens.erase(it);
			} else {
				it++;
			}
		}
		for(auto &cb : callbacks) {
			cb();
		}
	}
private:

	static double linear_fn(double t);
	static double smoothStep_fn(double t);
	static double easeInSine_fn (double t);
	static double easeOutSine_fn(double t);
	static double easeInOutSine_fn(double t);
	static double easeInBack_fn (double t);
	static double easeOutBack_fn(double t);
	static double easeInOutBack_fn(double t);

	static double sine_fn(double t);

	double delay;
	double startTime;
	double totalTime;
	double dspeed;

	bool do_rep = false;
	bool backto = false;

	std::vector<std::shared_ptr<TweenAttrBase>> args;
	std::function<double(double)> tweenFunc;
	std::function<void()> onCompleteFunc;

	static double currentTime;
public:
	static std::vector<std::shared_ptr<Tween>> allTweens;
};


Tween& make_tween();


} // namespace tween

#endif // TWEEN_H