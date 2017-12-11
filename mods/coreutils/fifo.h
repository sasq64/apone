#pragma once

#include <coreutils/log.h>
#include <coreutils/thread.h>

#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <chrono>
using namespace std::chrono_literals;

namespace utils {

template <typename T> class Fifo {

public:
	Fifo(int size) {
		bufSize = size;
		buffer = nullptr;
		if(size > 0) {
			buffer = new T [size];
		}
		bufPtr.store(buffer.load());
		wantToWrite = 0;
	}
	~Fifo() {
		// Wait for writers to finish
		quitting = true;
		while(wantToWrite > 0)
			cv.notify_all();
		if(buffer) {
			T* b = buffer.load();
			delete [] b;
		}
	}

	void quit() {
		quitting = true;
		while(wantToWrite > 0)
			cv.notify_all();
	}

	Fifo& operator=(Fifo&) = delete;
	Fifo(const Fifo&) = delete;

	void clear() {
		{
			std::unique_lock<std::mutex> lock(m);
			bufPtr.store(buffer);
		}
		cv.notify_all();
	}

	void put(const T *source, int count) {
		if(quitting)
			return;

		std::unique_lock<std::mutex> lock(m);
		while(left() < count && !quitting) {
			if(wantToWrite == 0)
				wantToWrite = count;
			cv.wait_for(lock, 100ms, [=] {
				return left() >= count || quitting;
			});
		}
		wantToWrite = 0;
		if(quitting)
			return;

		if(source)
			memcpy(bufPtr, source, sizeof(T) * count);
		bufPtr += count;
	}

	int get(T *target, int count) {
		if(quitting)
			return -1;

		{
			std::unique_lock<std::mutex> lock(m);
			int f = filled();
			if(count > f)
				count = f;

			memcpy(target, buffer, count * sizeof(T));
			if(f > count)
				memmove(buffer, &buffer[count], (f - count) * sizeof(T));
			bufPtr = &buffer[f - count];
		}

		if(left() >= wantToWrite)
			cv.notify_all();

		return count;
	}

	int filled() const {
	   	return bufPtr - buffer; 
	}
	int left() { 
		return bufSize - (bufPtr - buffer); 
	}
	int size() {
	   	return bufSize; 
	}
	T *ptr() { return bufPtr; }

protected:

	std::mutex m;
	std::condition_variable cv;

	std::atomic<int> wantToWrite;
	std::atomic<int> bufSize;
	std::atomic<int> position;
	std::atomic<T*> buffer;
	std::atomic<T*> bufPtr;
	std::atomic<bool> quitting{false};

};


template <typename T> class AudioFifo : public Fifo<T> {

public:
	AudioFifo(int size) : Fifo<T>(size) {
		volume = 1.0;
		lastSoundPos = position = 0;
	}

	void process(T *samples, int count) {

		int soundPos = -1;

		for(int i=0; i<count; i++) {
			short s = samples[i];
			if(s > 16 || s < -16)
				soundPos = i;
		}

		if(volume != 1.0) {
			for(int i=0; i<count; i++) {
				samples[i] = (samples[i] * volume);
			}
		}

		if(soundPos >= 0)
			lastSoundPos = position + soundPos;

		position += count;
	}

	void put(T *target, int count) {
		Fifo<T>::put(target, count);
		process(Fifo<T>::bufPtr - count, count);
	}

	void clear() {
		Fifo<T>::clear();
		volume = 1.0;
		lastSoundPos = position = 0;
	}

	int getSilence() { return position - lastSoundPos; }
	void setVolume(float v) { volume = v; }
	float getVolume() { return volume; }

private:
	float volume;

	int lastSoundPos;
	int position;

};

} // namespace utils

