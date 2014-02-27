#ifndef AUDIOPLAYER_LINUX_H
#define AUDIOPLAYER_LINUX_H

#include "audioplayer.h"

#include <linux/soundcard.h>
#include <alsa/asoundlib.h>

#include <thread>
#include <vector>

class InternalPlayer {
public:
	InternalPlayer(int hz = 44100) : quit(false) {
		init();
	}

	InternalPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100) : callback(cb), quit(false) {
		init();
		playerThread = std::thread {&InternalPlayer::run, this};

	}
	void init() {
		int err;
		if((err = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			fprintf(stderr, "cannot open audio device (%s)\n", snd_strerror(err));
			exit(1);
		}
		if((err = snd_pcm_set_params(playback_handle,
		          SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100, 1, 500000)) < 0) {
			fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
			exit(1);
		}
	}

	~InternalPlayer() {
		quit = true;
		if(playerThread.joinable())
			playerThread.join();
	}

	void run() {
		std::vector<int16_t> buffer(16384);
		while(!quit) {
			callback(&buffer[0], buffer.size());
			writeAudio(&buffer[0], buffer.size());
		}
	}


	void writeAudio(int16_t *samples, int sampleCount) {
		int frames = snd_pcm_writei(playback_handle, (char*)samples, sampleCount/2);
		if (frames < 0) {
			snd_pcm_recover(playback_handle, frames, 0);
		}
	}

	std::function<void(int16_t *, int)> callback;
	bool quit;
	int dspFD;
	snd_pcm_t *playback_handle;
	std::thread playerThread;

};

#endif // AUDIOPLAYER_LINUX_H
