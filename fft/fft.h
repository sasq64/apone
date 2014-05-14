
class FFT {
public:
	FFT(int size);
	void calc(float *in, float *out);

};


template <int FFT_SIZE = 2048, int SLOTS = 32> class SpectrumAnalyzer {
public:

	SpectrumAnalyzer(int startfreq, int endfreq

	void addAudio(int16_t *samples, int len);

	int msAvailable();

	std::array<int16_t, SLOTS> getFrequencies();

};


if(msAvailable() > 500)
	auto &f = getFrequencies();
	popFrequencies();