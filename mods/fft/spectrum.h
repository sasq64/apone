#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <vector>
#include <deque>
#include <coreutils/thread.h>
#include <array>

class SpectrumAnalyzer {
public:
	constexpr static int fft_size = 1024;
	constexpr static int eq_slots = 24;

	struct Internal;

private:

	std::deque<std::array<uint16_t, eq_slots>> spectrum;
	std::mutex m;
	std::vector<uint8_t> eq;
	std::vector<float> power;

	struct Internal *si;
public:

	SpectrumAnalyzer();
	~SpectrumAnalyzer();

	const std::array<uint16_t, eq_slots> getLevels() {
		std::lock_guard<std::mutex> guard(m);
		return spectrum.front();
	}

	int size() {
		std::lock_guard<std::mutex> guard(m);
		return spectrum.size();
	}

	void popLevels();

	void addAudio(int16_t *samples, int len);

};

#endif // SPECTRUM_H
