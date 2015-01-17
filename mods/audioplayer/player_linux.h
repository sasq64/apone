#ifndef AUDIOPLAYER_LINUX_H
#define AUDIOPLAYER_LINUX_H

#include "audioplayer.h"

#include <linux/soundcard.h>
#include <alsa/asoundlib.h>

#include <thread>
#include <vector>
#include <atomic>

class InternalPlayer {
public:
	InternalPlayer(int hz = 44100) : quit(false), playback_handle(nullptr), paused(false) {
	};

	InternalPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100) : callback(cb), quit(false), paused(false) {
		playerThread = std::thread {&InternalPlayer::run, this};
	}

	void pause(bool on) {
		paused = on;
	}

	~InternalPlayer() {
		quit = true;
		if(playerThread.joinable())
			playerThread.join();
		if(playback_handle)
			snd_pcm_close(playback_handle);
	}

	void run() {

		int err;
		if((err = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			fprintf(stderr, "cannot open audio device (%s)\n", snd_strerror(err));
			exit(1);
		}
		if((err = snd_pcm_set_params(playback_handle,
		          SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED, 2, 44100, 1, 30000)) < 0) {
			fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
			exit(1);
		}

		std::vector<int16_t> buffer(8192);
		while(!quit) {
			if(!paused) {
				callback(&buffer[0], buffer.size());
				writeAudio(&buffer[0], buffer.size());
			}
		}
		if(playback_handle)
			snd_pcm_close(playback_handle);
		playback_handle = nullptr;
	}

	void writeAudio(int16_t *samples, int sampleCount) {
		int frames = snd_pcm_writei(playback_handle, (char*)samples, sampleCount/2);
		if (frames < 0) {
			snd_pcm_recover(playback_handle, frames, 0);
		}
	}

	int get_delay() const{
	#ifdef RASPBERRYPI
		return 1;
	#else
		return 1;
	#endif
	}

	std::function<void(int16_t *, int)> callback;
	bool quit;
	int dspFD;
	snd_pcm_t *playback_handle;
	std::atomic<bool> paused;
	std::thread playerThread;

};

#endif // AUDIOPLAYER_LINUX_H
