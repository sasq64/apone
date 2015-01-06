#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

class audio_exception : public std::exception {
public:
	audio_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
	
	std::string msg;
};

class InternalPlayer;

class AudioPlayer {
public:
	AudioPlayer(int hz = 44100);
	AudioPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100);

	static void play(std::function<void(int16_t *, int)> cb, int hz = 44100);

	static void close();

	void pause();
	void resume();

	static void pause_audio(); 

	static void resume_audio();

	void touch() const {}

	//void writeAudio(int16_t *samples, int sampleCount) {
	//	internalPlayer->writeAudio(samples, sampleCount);
	//}

private:
		std::shared_ptr<InternalPlayer> internalPlayer;
		static std::shared_ptr<InternalPlayer> staticInternalPlayer;
};

#endif // AUDIOPLAYER_H
