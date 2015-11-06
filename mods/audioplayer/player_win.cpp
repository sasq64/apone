
#include "player_win.h"

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

void InternalPlayer::init() {

	blockCounter = 0;
	blockPosition = 0;
	bufSize = 32768;
	bufCount = 4;

	WAVEFORMATEX wfx;
	wfx.nSamplesPerSec = 44100;
	wfx.wBitsPerSample = 16;
	wfx.nChannels = 2;
	wfx.cbSize = 0; /* size of _extra_ info */
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)InternalPlayer::waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		fprintf(stderr, "unable to open WAVE_MAPPER device\n");
		ExitProcess(1);
	}

	InitializeCriticalSection(&lock);

	header.resize(bufCount);
	buffer.resize(bufCount);

	for(int i=0; i<bufCount; i++) {
		buffer[i].resize(bufSize);
		ZeroMemory(&header[i], sizeof(WAVEHDR));
		header[i].lpData = (LPSTR) &buffer[i][0];
	}

}

InternalPlayer::~InternalPlayer() {
	waveOutClose(hWaveOut);
}

void InternalPlayer::writeAudio(int16_t *samples, int sampleCount) {

		//printf("Writing block %d\n", blockPosition);

		WAVEHDR &h = header[blockPosition];
		h.dwBufferLength = sampleCount * 2;
		memcpy(h.lpData, samples, sampleCount * 2);

		if(h.dwFlags & WHDR_PREPARED) 
			waveOutUnprepareHeader(hWaveOut, &h, sizeof(WAVEHDR));

		waveOutPrepareHeader(hWaveOut, &h, sizeof(WAVEHDR));
		waveOutWrite(hWaveOut, &h, sizeof(WAVEHDR));

		EnterCriticalSection(&lock);
		blockCounter++;
		LeaveCriticalSection(&lock);

		blockPosition++;
		blockPosition %= bufCount;

		while(true) {
			EnterCriticalSection(&lock);
			int bc = blockCounter;
			LeaveCriticalSection(&lock);
			if(bc < bufCount)
				break;
			Sleep(100);
		}
	}

