#include "tween.h"

#define _USE_MATH_DEFINES
#include <cmath>

//namespace grappix {

namespace tween {

double Tween::linear_fn(double t, double delta) {
	return delta*t;
}

double Tween::smoothStep_fn(double t, double delta) {
	return delta*(t*t *(3 - 2*t));
}

double Tween::easeInSine_fn (double t, double delta) {
	return -delta * cos(t * (M_PI/2)) + delta;
}

double Tween::easeOutSine_fn(double t, double delta) {
	return delta * sin(t * (M_PI/2));        
}

double Tween::easeInOutSine_fn(double t, double delta) {
	return -delta/2 * (cos(M_PI*t) - 1);
}

double Tween::easeInBack_fn (double t, double delta) {
	double s = 1.70158f;
	return delta*t*t*((s+1)*t - s);
}

double Tween::easeOutBack_fn(double t, double delta) {        
	double s = 1.70158f;
	t--;
	return delta*(t*t*((s+1)*t + s) + 1);
}

double Tween::easeInOutBack_fn(double t, double delta) {
	double s = 1.70158f * 1.525f;
	//double s2 = s * 1.525f;
	t *= 2;
	if (t < 1) return delta/2*(t*t*((s+1)*t - s));
	double postFix = t-=2;
	return delta/2*((postFix)*t*((s+1)*t + s) + 2);
}

std::vector<std::shared_ptr<Tween>> Tween::allTweens;
double Tween::currentTime = 0;

Tween& make_tween() {
	Tween::allTweens.push_back(std::make_shared<Tween>());
	return *(Tween::allTweens.back());
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