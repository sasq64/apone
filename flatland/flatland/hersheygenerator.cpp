#include "hersheygenerator.h"
#include "primitive.h"
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

extern signed char simplex[];

//RIGHT_ARROW = 232
//501 - Uppercase
//SPADES = 741
//HEARTS = 742
//DIAMONDS = 743
//CLUBS = 744
//CIRCLE1=900
//CIRCLE8=907
//ANCHOR=860
//BELL=869
//PALM=870
//SHIELD1=908
//SHIELD2=909
//MUSIC=2317-2332,2367-2382
static int simplexA[] = {
		699,714,717,733,719,2271,734,731,
		721,722,2219,725,711,724,710,720,
		0,700,709,
		712,713,2241,726,2242,715,2273,
		0,501,526,
		2223,804,2224,2262,999,730,
		0,601,626,
		2225,723,2226,2246,718
};

namespace flatland {

std::map<int, signed char *> HersheyGenerator::offsets;
std::vector<short> HersheyGenerator::ascii;

void HersheyGenerator::init() {
	signed char *sptr = simplex;
	while(true) {
		if(sptr[0] == -1)
			break;
		int code = (sptr[0]<<8) | (sptr[1]+128);
		int verts = sptr[2]+128 - 1;
		HersheyGenerator::offsets[code] = sptr;
		//offsets.insert(make_pair(code, sptr));
		sptr += (5 + verts*2);
	}

	ascii.resize(256);
	int j = 0x20;
	for(unsigned int i=0; i<sizeof(simplexA)/sizeof(int); i++) {
		int c = simplexA[i];
		if(c != 0)
			ascii[j++] = c;
		else {
			int start = simplexA[++i];
			int end = simplexA[++i];
			while(start <= end)
				ascii[j++] = start++;
		}
	}
	ascii[0x10] = 741;
	ascii[0x11] = 742;
	ascii[0x12] = 743;
	ascii[0x13] = 744;
	printf("%d\n", j);
}

const Shape HersheyGenerator::createText(const char *text) {

	if(ascii.size() == 0)
		init();

	const char *ptr = text;
	float x = 0;
	Shape textShape;
	while(*ptr) {
		int code = ascii[*ptr];
		signed char *sptr = HersheyGenerator::offsets[code];
		int width = sptr[4] - sptr[3];
		Shape shape = createLetter(code).translate(vec2(x - sptr[3],0));
		textShape = textShape.concat(shape);
		x += width;
		ptr++;
	}
	return textShape;

}

bool HersheyGenerator::exists(int code) {
	return (HersheyGenerator::offsets[code] != nullptr);
}

const Shape HersheyGenerator::createLetter(int code) {

	if(ascii.size() == 0)
		init();

	signed char *sptr = HersheyGenerator::offsets[code];
	int verts = sptr[2]+128 - 1;
	sptr += 5;

	vector<vec2> points;
	Shape shape;

	signed char *first = sptr;
	for(int j=0; j<verts; j++) {

		//printf("%d: %d %d\n", j, sptr[0], sptr[1]);
		if(sptr[0] == 127 && sptr[1] == 127) {
			int flags = 0;
			if(sptr[-2] == first[0] && sptr[-1] == first[1]) {
				flags = Primitive::FLAG_ISLOOP;
				points.erase(points.end());
			}
			shape = shape.concat(Shape(points, flags));
			points.clear();
			first = sptr + 2;
		} else {
			vec2 p = vec2(sptr[0], -sptr[1]);
			points.push_back(p);
		}
		sptr += 2;
	}
	int flags = 0;
	if(sptr[-2] == first[0] && sptr[-1] == first[1]) {
		flags = Primitive::FLAG_ISLOOP;
		points.erase(points.end());
	}
	shape = shape.concat(Shape(points, flags));

	return shape;


/*
		sptr += 5;

		verts -= 1;

		printf("%d : %d verts, %d spacing\n", code, verts, spc);
		CompositeShape *letter = nullptr;
		//Shape *part = new Shape();
		for(int j=0; j<verts; j++) {
			//vec2 p = vec2(simplex[i][2+j*2], simplex[i][2+j*2+1]);
			vec2 p = vec2(sptr[0], -sptr[1]);
			sptr += 2;


			if(p.x == 127 && p.y == -127) {
				if(partPoints.size() > 0) {
					if(letter == nullptr)
						letter = new CompositeShape();
					letter->addShape(Shape::fromPoints(partPoints));
					partPoints.clear();
				}
			} else
				partPoints.push_back(p);
		}
		//printf("Adding %d at x %d\n", i, pos.x);
		//font->add(ol, 0, pos);
		if(letter) {
			letter->addShape(Shape::fromPoints(partPoints));
			//letter->addShape(part);
			letters[code] = letter;

			//delete letter;

		} else {
			letters[code] = Shape::fromPoints(partPoints);
		}
		partPoints.clear();
		//pos.x += spacing;
		spacing[code] = spc;
	}
*/

}


} /* namespace flatland */
