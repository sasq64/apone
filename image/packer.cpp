
#include "packer.h"


#if (defined UNIT_TEST) || (defined IMAGE_UNIT_TEST)

#include <catch.hpp>

TEST_CASE("image::packer", "packer") {

	using namespace std;
	using namespace image;
	Packer p({0,0,100,100});

	Packer::Rect r { 0,0,20,20 };
	p.add(r);
	REQUIRE(r.x == 0);
	REQUIRE(r.y == 0);

}

#endif
