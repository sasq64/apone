#include "spectrum.h"

#include <cmath>

using namespace std;

void SpectrumAnalyzer::popLevels() {
	lock_guard<mutex> guard(m);
	spectrum.pop_front();
}

void SpectrumAnalyzer::addAudio(int16_t *samples, int len) {

	static vector<float> power(fft_size/2+1);

	int pos = 0;

	while(pos < len) {
		for(int i=0; i<fft_size; i++) {
			fftin[i] = ((float)samples[pos+i*2] + (float)samples[pos+i*2+1])/(65536.0);
		}
		//rfftw_one(fftwp, fftin, fftout);
		fftw_execute(fftwp);

		pos += fft_size*2;

		power[0] = fftout[0][0]*fftout[0][0];  /* DC component */
		for(int k = 1; k < (fft_size+1)/2; ++k)  /* (k < fft_size/2 rounded up) */
			power[k] = fftout[0][k]*fftout[0][k] + fftout[0][fft_size-k]*fftout[0][fft_size-k];
		power[fft_size/2] = fftout[0][fft_size/2]*fftout[0][fft_size/2];  /* Nyquist freq. */
		float sum = 0;
		int j = 0;

		auto d =  pow(10.0, (3.0/(10.0 * 3)));

		float freq_per_slot = 44100.0/fft_size;

		float start_freq = 22.0;
		float next_freq = start_freq * d;

		{ lock_guard<mutex> guard(m);
			spectrum.emplace_back();
			auto &spec = spectrum.back();
			float curr_freq = 0;
			//LOGD("----------- STEP %f", freq_per_slot);
			for(int i=1; i<fft_size/2; i++) {

				if(curr_freq >= start_freq)
					sum += power[i];

				if(curr_freq >= next_freq) {
					//LOGD("%d (%d). [%f] %f\n", j, i, next_freq, sum);
					spec[j++] = sqrt(sum);
					if(j == eq_slots)
						break;
					sum = 0;
					next_freq *= d;
				}
				curr_freq += freq_per_slot;

			}
		}
	}
}
