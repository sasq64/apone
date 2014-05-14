#include <catch.hpp>

#include "CMP_fft.h"


using namespace std;

TEST_CASE("utils::fft", "fft") {

	//REQUIRE(4 == 4);
	vector<short> output(1024-1);
	vector<short> input(2048);
	for(int i=0; i<2048; i++) {
		input[i] = sinf(i*2*M_PI/256.0) * 8191;
		//input[i] += sinf((i+200)*2*M_PI/256.0) * 8191;
		input[i] += sinf((i+500)*2*M_PI/123.0) * 8191;
	}

	fft(&input[0], &output[1], 2048);

	for(int i=0; i<1024-1; i++) {
		if(output[i] != 0)
			printf("%04d: %d\n", i, output[i]);
	}

}
