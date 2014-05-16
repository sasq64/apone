#ifndef TWEEN_H
#define TWEEN_H

#include <coreutils/log.h>

#include <cmath>
#include <functional>
#include <vector>
#include <limits>
#include <memory>

//namespace grappix {

namespace tween {

double linear(double t, double s);
double smoothStep(double t, double s);
double easeInSine (double t, double b);
double easeOutSine(double t, double b);
double easeInOutSine(double t, double b);
double easeInBack (double t, double b);
double easeOutBack(double t, double b);
double easeInOutBack(double t, double b);

class TweenAttrBase {
public:

	TweenAttrBase(double target, double value) : startValue(target), delta(value - target), maxValue(std::numeric_limits<double>::max()) {}

	virtual void set(double v) = 0;

	double startValue;
	double delta;
	double delay;

	double maxValue;

	std::function<void()> func;
	std::function<double(double, double)> tweenFunc;

};


template <typename T> class TweenAttr : public TweenAttrBase {
public:
	TweenAttr(T &target, T value) : TweenAttrBase(target, value), target(&target) {}
	T *target;

	virtual void set(double v) override {
		*target = static_cast<T>(v);
	}
};

class Tween;

class Holder {
public:
	Holder() {}
	Holder(std::shared_ptr<Tween> t) : tween(t) {}
	void cancel();
	bool done();
	void finish();
private:
	std::shared_ptr<Tween> tween;
};

class Tween {
public:
	Tween() : delay(0.0), startTime(currentTime), tweenFunc(smoothStep) {}

	Tween& operator=(const Tween &other) = delete;
	Tween(const Tween& other) = delete;

	operator Holder() {
		return Holder(Tween::allTweens.back());
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
		tweenFunc = tween::linear;
		return *this;
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	Tween& to(T &target, T value) {
		for(int i=0; i<target.size(); i++) {
			args.emplace_back(std::make_shared<TweenAttr<T>>(target[i], value[i]));
			auto &a = args.back();
			a->tweenFunc = tweenFunc;
			a->delay = delay;
		}
		return *this;
	}

	template <typename T, typename U> Tween& to(T &target, U value, int cycles = 1) {
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

		LOGD("Negating delta %f and startValue %f -> %f", a->delta, a->startValue, a->startValue + a->delta);
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
				ended++;
				a->set(a->startValue + fmod(a->delta, a->maxValue));
				continue;
			}
			a->set(a->startValue + fmod(a->tweenFunc(t, a->delta), a->maxValue));
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
				//allTweens.erase(it++);
				it = allTweens.erase(it);
			} else {
				//LOGD("Tween going");
				it++;
			}
		}
		for(auto &cb : callbacks) {
			cb();
		}
	}

	double delay;
	double startTime;
	double totalTime;
	double dspeed;

	std::vector<std::shared_ptr<TweenAttrBase>> args;
	std::function<double(double, double)> tweenFunc;
	std::function<void()> onCompleteFunc;

	static double currentTime;
	static std::vector<std::shared_ptr<Tween>> allTweens;
};

//int to(float totalTime, const std::initializer_list<TweenAttr> &il);


Tween& make_tween();


}
//}

#endif // TWEEN_H