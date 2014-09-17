#include "tween.h"

namespace tween {

std::vector<std::shared_ptr<Tween>> Tween::allTweens;
double Tween::currentTime = 0;
std::mutex Tween::tweenMutex;

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

Tween Tween::make() {
	Tween::allTweens.push_back(std::make_shared<Tween>());
	return *(Tween::allTweens.back());
}

void Tween::cancel() {

	auto it = Tween::allTweens.begin();

	while(it != Tween::allTweens.end()) {
		if((*it)->impl == impl) {
			LOGD("Tween canceled");
			Tween::allTweens.erase(it);
			break;
		}
		++it;
	}
}

bool Tween::step() {
		size_t ended = 0;
		for(auto &a : impl->args) {
			float t = (float)((currentTime - impl->startTime - a->delay) / impl->totalTime);
			if(t < 0.0)
				continue;
			if(t > 1.0) {
				if(impl->do_rep)
					t -= 1.0;
				else if(impl->backto) {
					ended++;
					a->set(0.0, *this);
				} else {
					ended++;
					a->set(1.0, *this);
					continue;
				}
			}
			a->set(impl->tween_func(t), *this);
		}
		return ended < impl->args.size();
	}

void Tween::updateTweens(double t) {

	static std::vector<std::shared_ptr<Tween>> doneTweens;

	std::lock_guard<std::mutex> guard(tweenMutex);

	currentTime = t;
	auto it = allTweens.begin();
	while(it != allTweens.end()) {
		if(!(*it)->step()) {
			doneTweens.push_back(*it);
			it = allTweens.erase(it);
		} else {
			it++;
		}
	}

	for(auto &dt : doneTweens) {
		dt->impl->on_complete_cb.call(*dt, 1.0);
	}
	doneTweens.clear();
}

void Tween::finish() {
	for(auto &a : impl->args) {
		a->set(1.0, *this);
	}
	cancel();
}

Tween &Tween::seconds(float s) {
	impl->totalTime = s;
	return *this;
}

Tween &Tween::speed(float s) {
	impl->dspeed = s;
	return *this;
}

Tween &Tween::linear() {
	impl->tween_func = linear_fn;
	return *this;
}

Tween &Tween::smoothstep() {
	impl->tween_func = smoothStep_fn;
	return *this;
}

Tween &Tween::easeinback() {
	impl->tween_func = easeInBack_fn;
	return *this;
}

Tween &Tween::easeoutback() {
	impl->tween_func = easeOutBack_fn;
	return *this;
}

Tween &Tween::easeinsine() {
	impl->tween_func = easeInSine_fn;
	return *this;
}

Tween &Tween::easeoutsine() {
	impl->tween_func = easeOutSine_fn;
	return *this;
}

Tween &Tween::sine() {
	impl->tween_func = sine_fn;
	impl->backto = true;
	return *this;
}

Tween &Tween::repeating() {
	impl->do_rep = true;
	return *this;
}
} // namespace tween

#if (defined UNIT_TEST || defined TWEEN_UNIT_TEST)

#include "catch.hpp"
#include <coreutils/utils.h>
#include <stdio.h>
TEST_CASE("tween::basic", "Basic tween") {

	using tween::Tween;

	struct { int score = 0; short energy = 0; } demo;

	auto showScore = [&](int score) {
		fprintf(stderr, "SCORE:%d\n", score);
	};

	auto showFood = [](int food, Tween t, double v) -> int {
		fprintf(stderr, "FOOD:%d (%f)\n", food, v);
		return 0;
	};

	Tween::make().linear().to(demo.score, 10).onUpdate(showScore).from(demo.energy, 250).onUpdate(showFood).seconds(2);
	//.onUpdate(showScore);

	double t = 0;
	for(int i=0; i<10; ++i) {
		Tween::updateTweens(t += 0.1);
	}
	REQUIRE(demo.score == 5);
	REQUIRE(demo.energy == 125);
	for(int i=0; i<50; ++i)
		Tween::updateTweens(t += 0.1);

	REQUIRE(demo.score == 10);
	REQUIRE(demo.energy == 0);

	std::vector<float> v = { 0, 1, 10, 100 };
	Tween h = Tween::make().to(v, {4,4,4,4}).seconds(4.0);
	for(int i=0; i<10; ++i)
		Tween::updateTweens(t += 0.1);
	REQUIRE(v[0] == 0.625);
	REQUIRE(v[3] == 85);
	for(int i=0; i<30; ++i)
		Tween::updateTweens(t += 0.1);
	REQUIRE(v[1] == 4);
	REQUIRE(v[2] == 4);

	Tween::make().fromTo(10,20).onUpdate([](int x) { LOGD("%d", x); }).seconds(1);
	for(int i=0; i<20; ++i)
		Tween::updateTweens(t += 0.1);


}

#endif