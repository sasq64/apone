#ifndef AUDIOPLAYER_WINDOWS_H
#define AUDIOPLAYER_WINDOWS_H

#include "audioplayer.h"

#include <vector>
#include <stdint.h>
#include <windows.h>
#include <mmsystem.h>


class InternalPlayer {
public:
	InternalPlayer(int hz = 44100) : quit(false) {
		init();
	}

	InternalPlayer(std::function<void(int16_t *, int)> cb, int hz = 44100) : callback(cb), quit(false) {
		init();
	}

	~InternalPlayer();
	
	void init();
	void writeAudio(int16_t *samples, int sampleCount);

	void pause(bool on) {
	}

	int get_delay() const { return 1; }


private:

	bool quit;
	std::function<void(int16_t *, int)> callback;


	CRITICAL_SECTION lock;
	volatile int blockCounter;
	volatile int blockPosition;

	HWAVEOUT hWaveOut;

	int bufSize;
	int bufCount;

	std::vector<std::vector<int16_t>> buffer;
	std::vector<WAVEHDR> header;

	static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {

		if(uMsg != WOM_DONE)
			return;

		InternalPlayer *ap = (InternalPlayer*)dwInstance;
		EnterCriticalSection(&ap->lock);
		ap->blockCounter--;
		LeaveCriticalSection(&ap->lock);
	}

};

//typedef AudioPlayerWindows AudioPlayerNative;

#endif // AUDIOPLAYER_WINDOWS_H