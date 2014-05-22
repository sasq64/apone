#include "tween.h"

#define _USE_MATH_DEFINES
#include <cmath>

//namespace grappix {

namespace tween {

double Tween::linear_fn(double t) {
	return t;
}

double Tween::smoothStep_fn(double t) {
	return (t*t *(3 - 2*t));
}

double Tween::easeInSine_fn (double t) {
	return 1 - cos(t * (M_PI/2));
}

double Tween::sine_fn(double t) {
	return (sin(t * (M_PI*2) - M_PI/2) + 1.0)/2.0;        
}

double Tween::easeOutSine_fn(double t) {
	return sin(t * (M_PI/2));        
}

double Tween::easeInOutSine_fn(double t) {
	return -0.5 * (cos(M_PI*t) - 1);
}

double Tween::easeInBack_fn (double t) {
	double s = 1.70158f;
	return t*t*((s+1)*t - s);
}

double Tween::easeOutBack_fn(double t) {        
	double s = 1.70158f;
	t--;
	return (t*t*((s+1)*t + s) + 1);
}

double Tween::easeInOutBack_fn(double t) {
	double s = 1.70158f * 1.525f;
	//double s2 = s * 1.525f;
	t *= 2;
	if (t < 1) return 1.0/2*(t*t*((s+1)*t - s));
	double postFix = t-=2;
	return 1.0/2*((postFix)*t*((s+1)*t + s) + 2);
}

std::vector<std::shared_ptr<Tween>> Tween::allTweens;
double Tween::currentTime = 0;

Tween& make_tween() {
	Tween::allTweens.push_back(std::make_shared<Tween>());
	return *(Tween::allTweens.back());
}

bool TweenHolder::valid() {
	return tween != nullptr;
}

void TweenHolder::cancel() {
	if(!tween) return;
	auto it = Tween::allTweens.begin();
	//LOGD("We have %d tweens", allTweens.size());
	while(it != Tween::allTweens.end()) {
		if(*it == tween) {
			LOGD("Tween canceled");
			Tween::allTweens.erase(it);
			break;
		}
		++it;
	}
}

void TweenHolder::finish() {
	if(!tween) return;
	tween->finish();
	auto it = Tween::allTweens.begin();
	while(it != Tween::allTweens.end()) {
		if(*it == tween) {
			LOGD("Tween finished");
			Tween::allTweens.erase(it);
			break;
		}
		++it;
	}
}

//}
}