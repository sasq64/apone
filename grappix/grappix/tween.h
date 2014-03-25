#ifndef TWEEN_H
#define TWEEN_H

#include <coreutils/log.h>

#include <functional>
#include <vector>

namespace grappix {

namespace tween {

enum TweenFuncArg {
	ease = 0
};

enum DoubleArg {
	delay = 10 
};

enum FuncArg {
	onComplete = 20
};

double linear(double t, double s, double delta);
double smoothStep(double t, double s, double delta);
double easeInSine (double t, double b , double c);
double easeOutSine(double t, double b ,double c);
double easeInOutSine(double t, double b ,double c);
double easeInBack (double t,double b , double c);
double easeOutBack(double t,double b , double c);       
double easeInOutBack(double t,double b , double c);

class TweenAttr {
public:
	TweenAttr(float &target, float value) : type(FLOAT), ftarget(&target), startValue(target), delta(value - target) {}
	TweenAttr(double &target, double value) : type(DOUBLE), dtarget(&target), startValue(target), delta(value - target) {}
	TweenAttr(int &target, int value) : type(INT), itarget(&target), startValue(target), delta(value - target) {}
	TweenAttr(FuncArg ta, std::function<void()> f) : type(TWEENARG), arg(ta), func(f) {}
	TweenAttr(TweenFuncArg ta, std::function<double(double, double, double)> f) : type(TWEENARG), arg(ta), tweenFunc(f) {}
	TweenAttr(DoubleArg ta, double d) : type(TWEENARG), arg(ta), startValue(d) {}

	enum Type {
		//BYTE,
		//SHORT,
		INT,
		FLOAT,
		DOUBLE,
		TWEENARG
	};

	Type type;
	union {
		float *ftarget;
		double *dtarget;
		int *itarget;
		int arg;
	};

	double startValue;
	double delta;

	double delay;

	std::function<void()> func;
	std::function<double(double, double, double)> tweenFunc;

};

class Tween {
public:
	Tween() : delay(0.0), tweenFunc(smoothStep) {}

	static int to(float totalTime, const std::initializer_list<TweenAttr> &il) {
		Tween t;
		t.startTime = currentTime;
		t.totalTime = totalTime;
		for(auto &i : il) {
			t.add(i);
		}
		LOGD("Pushing tween");
		allTweens.push_back(t);
		return allTweens.size()-1;
	}

	static int from(float totalTime, const std::initializer_list<TweenAttr> &il) {
		Tween t;
		t.startTime = currentTime;
		t.totalTime = totalTime;
		for(auto &i : il) {
			t.add(i, true);
		}
		LOGD("Pushing tween");
		allTweens.push_back(t);
		return allTweens.size()-1;
	}

	static void update(int t, float totalTime, const std::initializer_list<TweenAttr> &il) {
		//Tween &tween = allTweens[t];
	}

	template <class... T> static int to(float totalTime, T ... args) {
		return toX(totalTime, { args... });
	}

	void add(const TweenAttr &ta, bool from = false) {

		if(ta.type == TweenAttr::TWEENARG) {
			switch(ta.arg) {
			case ease:
				tweenFunc = ta.tweenFunc;
				break;
			case onComplete:
				onCompleteFunc = ta.func;
				break;
			case tween::delay:
				this->delay = ta.startValue;
				break;
			}
			return;
		}

		args.push_back(ta);
		auto &a = args.back();
		a.tweenFunc = tweenFunc;
		a.delay = delay;
		if(from) {
			LOGD("Negating delta %f and startValue %f -> %f", a.delta, a.startValue, a.startValue + a.delta);
			//startValue(target), delta(value - target)
			a.startValue += a.delta;
			a.delta = -a.delta;
			switch(a.type) {
			case TweenAttr::FLOAT:
				*a.ftarget = a.startValue;
				break;
			case TweenAttr::DOUBLE:
				*a.dtarget = a.startValue;
				break;
			case TweenAttr::INT:
				*a.itarget = (int)a.startValue;
				break;
			case TweenAttr::TWEENARG:
				throw std::exception();//"Incorrect arg");
			}
		}
	}

	bool step() { // t : 0 -> 1
		size_t ended = 0;
		for(auto &a : args) {
			float t = (currentTime - startTime - a.delay) / totalTime;
			if(t < 0.0)
				continue;
			if(t > 1.0) {
				ended++;
				switch(a.type) {
				case TweenAttr::FLOAT:
					*a.ftarget = a.startValue + a.delta;
					break;
				case TweenAttr::DOUBLE:
					*a.dtarget = a.startValue + a.delta;
					break;
				case TweenAttr::INT:
					*a.itarget = (int)(a.startValue + a.delta);
					break;
				case TweenAttr::TWEENARG:
					throw std::exception();//"Incorrect arg");
				}
				continue;
			}
			switch(a.type) {
			case TweenAttr::FLOAT:
				*a.ftarget = a.tweenFunc(t, a.startValue, a.delta);
				break;
			case TweenAttr::DOUBLE:
				*a.dtarget = a.tweenFunc(t, a.startValue, a.delta);
				break;
			case TweenAttr::INT:
				*a.itarget = (int)a.tweenFunc(t, a.startValue, a.delta);
				break;
			case TweenAttr::TWEENARG:
				throw std::exception();//"Incorrect arg");
			}
		}
		return ended < args.size();
	};

	static void updateTweens(double t) {
		//std::vector<std::function<void()>> callbacks;
		currentTime = t;
		auto it = allTweens.begin();
		//LOGD("We have %d tweens", allTweens.size());
		while(it != allTweens.end()) {
			if(!it->step()) {
				LOGD("Tween ended");
				if(it->onCompleteFunc)
					it->onCompleteFunc();
				//allTweens.erase(it++);
				it = allTweens.erase(it);
			} else {
				//LOGD("Tween going");
				it++;
			}
		}
	}

	double startTime;
	double totalTime;

	double delay;

	std::vector<TweenAttr> args;
	std::function<double(double, double, double)> tweenFunc;
	std::function<void()> onCompleteFunc;

	static double currentTime;
	static std::vector<Tween> allTweens;
};


class NewTween {
public:
	NewTween() : delay(0.0), startTime(currentTime), tweenFunc(smoothStep) {}

	NewTween& seconds(float s) {
		totalTime = s;
		return *this;
	}

	NewTween& speed(float s) {
		dspeed = s;
		return *this;
	}

	NewTween& linear() {
		tweenFunc = tween::linear;
		return *this;
	}

	NewTween& to(float &target, float value) {
		args.emplace_back(target, value);
		auto &a = args.back();
		a.tweenFunc = tweenFunc;
		a.delay = delay;

		//a.startValue += a.delta;
		//a.delta = -a.delta;
		//*a.ftarget = a.startValue;
		return *this;
	}

	template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	NewTween& to(T &target, T value) {
		for(int i=0; i<2; i++) {
			args.emplace_back(target[i], value[i]);
			auto &a = args.back();
			a.tweenFunc = tweenFunc;
			a.delay = delay;
		}

		//a.startValue += a.delta;
		//a.delta = -a.delta;
		//*a.ftarget = a.startValue;
		return *this;
	}

	NewTween& on_complete(std::function<void()> f) {
		onCompleteFunc = f;
		return *this;
	}

/*
	static int to(float totalTime, const std::initializer_list<TweenAttr> &il) {
		NewTween t;
		t.startTime = currentTime;
		t.totalTime = totalTime;
		for(auto &i : il) {
			t.add(i);
		}
		LOGD("Pushing tween");
		allTweens.push_back(t);
		return allTweens.size()-1;
	}

	static int from(float totalTime, const std::initializer_list<TweenAttr> &il) {
		NewTween t;
		t.startTime = currentTime;
		t.totalTime = totalTime;
		for(auto &i : il) {
			t.add(i, true);
		}
		LOGD("Pushing tween");
		allTweens.push_back(t);
		return allTweens.size()-1;
	}

	static void update(int t, float totalTime, const std::initializer_list<TweenAttr> &il) {
		//Tween &tween = allTweens[t];
	}

	template <class... T> static int to(float totalTime, T ... args) {
		return toX(totalTime, { args... });
	}
*/


	void add(const TweenAttr &ta, bool from = false) {

		if(ta.type == TweenAttr::TWEENARG) {
			switch(ta.arg) {
			case ease:
				tweenFunc = ta.tweenFunc;
				break;
			case onComplete:
				onCompleteFunc = ta.func;
				break;
			case tween::delay:
				this->delay = ta.startValue;
				break;
			}
			return;
		}

		args.push_back(ta);
		auto &a = args.back();
		a.tweenFunc = tweenFunc;
		a.delay = delay;
		if(from) {
			LOGD("Negating delta %f and startValue %f -> %f", a.delta, a.startValue, a.startValue + a.delta);
			//startValue(target), delta(value - target)
			a.startValue += a.delta;
			a.delta = -a.delta;
			switch(a.type) {
			case TweenAttr::FLOAT:
				*a.ftarget = a.startValue;
				break;
			case TweenAttr::DOUBLE:
				*a.dtarget = a.startValue;
				break;
			case TweenAttr::INT:
				*a.itarget = (int)a.startValue;
				break;
			case TweenAttr::TWEENARG:
				throw std::exception();//"Incorrect arg");
			}
		}
	}

	bool step() { // t : 0 -> 1
		size_t ended = 0;
		for(auto &a : args) {
			float t = (currentTime - startTime - a.delay) / totalTime;
			if(t < 0.0)
				continue;
			if(t > 1.0) {
				ended++;
				switch(a.type) {
				case TweenAttr::FLOAT:
					*a.ftarget = a.startValue + a.delta;
					break;
				case TweenAttr::DOUBLE:
					*a.dtarget = a.startValue + a.delta;
					break;
				case TweenAttr::INT:
					*a.itarget = (int)(a.startValue + a.delta);
					break;
				case TweenAttr::TWEENARG:
					throw std::exception();//"Incorrect arg");
				}
				continue;
			}
			switch(a.type) {
			case TweenAttr::FLOAT:
				*a.ftarget = a.tweenFunc(t, a.startValue, a.delta);
				break;
			case TweenAttr::DOUBLE:
				*a.dtarget = a.tweenFunc(t, a.startValue, a.delta);
				break;
			case TweenAttr::INT:
				*a.itarget = (int)a.tweenFunc(t, a.startValue, a.delta);
				break;
			case TweenAttr::TWEENARG:
				throw std::exception();//"Incorrect arg");
			}
		}
		return ended < args.size();
	};

	static void updateTweens(double t) {
		std::vector<std::function<void()>> callbacks;
		currentTime = t;
		auto it = allTweens.begin();
		//LOGD("We have %d tweens", allTweens.size());
		while(it != allTweens.end()) {
			if(!it->step()) {
				LOGD("Tween ended");
				if(it->onCompleteFunc)
					callbacks.push_back(it->onCompleteFunc);
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

	std::vector<TweenAttr> args;
	std::function<double(double, double, double)> tweenFunc;
	std::function<void()> onCompleteFunc;

	static double currentTime;
	static std::vector<NewTween> allTweens;
};

int to(float totalTime, const std::initializer_list<TweenAttr> &il);


NewTween& make_tween();


}
}

#endif // TWEEN_H