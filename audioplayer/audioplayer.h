#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <functional>
#include <memory>
#include <stdexcept>

class audio_exception : public std::exception {
public:
	audio_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
	
	std::string msg;
};

#ifdef WIN32
#include "player_windows.h"
#elif defined LINUX
#include "player_linux.h"
#elif defined ANDROID
#include "player_sl.h"
#else
#include "player_sdl.h"
#endif


class AudioPlayer {
public:
	AudioPlayer(int hz = 44100) : internalPlayer(std::make_shared<InternalPlayer>(hz)) {}
	AudioPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100) : internalPlayer(std::make_shared<InternalPlayer>(cb, hz)) {
		staticInternalPlayer = internalPlayer;
	}

	void writeAudio(int16_t *samples, int sampleCount) {
		internalPlayer->writeAudio(samples, sampleCount);
	}
private:
		std::shared_ptr<InternalPlayer> internalPlayer;
		static std::shared_ptr<InternalPlayer> staticInternalPlayer;
};

#endif // AUDIOPLAYER_H
