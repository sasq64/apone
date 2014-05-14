#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <vector>
#include <deque>
#include <mutex>
#include <array>

#include <rfftw.h>


class SpectrumAnalyzer {
public:
	const static int fft_size = 1024;
	const static int eq_slots = 24;

private:
	std::deque<std::array<uint16_t, eq_slots>> spectrum;
	std::mutex m;

	std::vector<uint8_t> eq;

	fftw_real fftin[fft_size], fftout[fft_size];
	rfftw_plan fftwp;

public:

	SpectrumAnalyzer() : eq(eq_slots) {
		fftwp = rfftw_create_plan(fft_size, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE);
	}

	const std::array<uint16_t, eq_slots> getLevels() {
		return spectrum.front();
	}

	int size() {
		return spectrum.size();
	}

	void popLevels();

	void addAudio(int16_t *samples, int len);

};

#endif // SPECTRUM_H
