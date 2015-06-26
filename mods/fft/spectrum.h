#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <vector>
#include <deque>
#include <mutex>
#include <array>

//#include <rfftw.h>
//#include <fftw3.h>
//#include "fftw-3.3.4/api/fftw3.h"


class SpectrumAnalyzer {
public:
	const static int fft_size = 1024;
	const static int eq_slots = 24;

	struct Internal;

private:


	std::deque<std::array<uint16_t, eq_slots>> spectrum;
	std::mutex m;

	std::vector<uint8_t> eq;

	std::vector<float> power;

	//fftw_real fftin[fft_size], fftout[fft_size];
	//float fftin[fft_size];
	struct Internal *si;
	//fftwf_complex fftout[fft_size];
	//fftwf_plan fftwp;
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
