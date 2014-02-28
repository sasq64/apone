#ifndef PLAYER_SL_H
#define PLAYER_SL_H

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

class sl_exception : public std::exception {
public:
	sl_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

class InternalPlayer {
public:
	InternalPlayer(int hz = 44100) : quit(false) {
		init();
	}

	InternalPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100) : callback(cb), quit(false) {
		init();
	}

	void init();
private:

	static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context);

	std::function<void(int16_t *, int)> callback;
	bool quit;

	std::vector<int16_t> buffer;

	SLObjectItf engineObject;
	SLEngineItf engineEngine;
	SLObjectItf outputMixObject;

	SLObjectItf bqPlayerObject;
	SLPlayItf bqPlayerPlay;
	SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
	//SLEffectSendItf bqPlayerEffectSend;
};

#endif // PLAYER_SL_H
