#include "tween.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace grappix {

namespace tween {

double linear(double t, double s, double delta) {
	return delta*t + s;
}

double smoothStep(double t, double s, double delta) {
	return s + delta*(t*t *(3 - 2*t));
}

double easeInSine (double t, double b , double c) {
	return -c * cos(t * (M_PI/2)) + c + b;
}

double easeOutSine(double t, double b ,double c) {
	return c * sin(t * (M_PI/2)) + b;        
}

double easeInOutSine(double t, double b ,double c) {
	return -c/2 * (cos(M_PI*t) - 1) + b;
}

double easeInBack (double t,double b , double c) {
	double s = 1.70158f;
	return c*t*t*((s+1)*t - s) + b;
}

double easeOutBack(double t,double b , double c) {        
	double s = 1.70158f;
	t--;
	return c*(t*t*((s+1)*t + s) + 1) + b;
}

double easeInOutBack(double t,double b , double c) {
	double s = 1.70158f * 1.525f;
	//double s2 = s * 1.525f;
	t *= 2;
	if (t < 1) return c/2*(t*t*((s+1)*t - s)) + b;
	double postFix = t-=2;
	return c/2*((postFix)*t*((s+1)*t + s) + 2) + b;
}

std::vector<Tween> Tween::allTweens;
double Tween::currentTime = 0;

int to(float totalTime, const std::initializer_list<TweenAttr> &il) {
	return Tween::to(totalTime, il);
}

}
}