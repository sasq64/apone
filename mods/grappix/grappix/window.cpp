
#include "window.h"
#include "GL_Header.h"

namespace grappix {


void Window::resize(int w, int h) {
	_width = w;
	_height = h;
	update_matrix();
}

void Window::setup(int w, int h) {

	utils::perform_callbacks();

	if(winOpen)
		return;

	_width = w;
	_height = h;

	LOGD("Window %dx%d", _width, _height);

	update_matrix();

	glLineWidth(2.0);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	//glBlendColor(1.0,1.0,1.0,1.0);
	lastTime = -1;
	winOpen = true;

	startTime = chrono::high_resolution_clock::now();
	frameBuffer = 0;
}

int Window::call_repeatedly(std::function<void(void)> f, int msec) {
	callbacks.push_back(Callback(f, msec));
	return callbacks.size()-1;
}

void Window::update_callbacks() {

	auto t = chrono::high_resolution_clock::now();
	uint32_t us = chrono::duration_cast<chrono::microseconds>(t - startTime).count();
	tween::Tween::updateTweens(us / 1000000.0f);
	Resources::getInstance().update();

	utils::perform_callbacks();

	while(safeFuncs.size() > 0) {
		safeMutex.lock();
		auto &f = safeFuncs.front();
		f();
		safeFuncs.pop_front();
		safeMutex.unlock();
	}

	us = utils::getms();
	for(auto &cb : callbacks) {
		if(cb.msec == 0 || us >= cb.next_time) {
			cb.cb();
			cb.next_time += cb.msec;
		}
	}

	for(auto i : to_remove) {
		callbacks.erase(callbacks.begin() + i);
	}
	to_remove.clear();

	Program::frame_counter++;

}

void Window::remove_repeating(int index) {
	to_remove.insert(index);
}

void Window::benchmark() {
	benchStart = chrono::high_resolution_clock::now();
	bmCounter = 100;
}

void Window::run_safely(std::function<void()> f) {
	safeMutex.lock();
	safeFuncs.push_back(f);
	safeMutex.unlock();
	while(safeFuncs.size() != 0) {
		safeMutex.lock();
		safeMutex.unlock();
	}
}

}