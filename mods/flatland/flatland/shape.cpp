#include "GL_Header.h"
#include <vector>
#include <algorithm>
#include <ctype.h>
#include <glm/gtx/spline.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
//#include "glutil.h"
#include "shape.h"
#include "stepsequence.h"
#include "ConvexDecomposition/b2Polygon.h"
#include "hersheygenerator.h"

using namespace glm;
using namespace std;
//using namespace glutil;

namespace flatland {

Shape::Shape() : flags(0), primitives(make_shared<vector<Primitive>>()) {
}

Shape::Shape(const shared_ptr<const vector<Primitive>> &primitives, int flags) :  flags(flags), primitives(primitives) {
	calcMinMax();
}

Shape::Shape(const vector<vec2> &points, int flags) :  flags(flags),  primitives(make_shared<vector<Primitive>>( initializer_list<Primitive>{ Primitive(points, flags) } )) {
	calcMinMax();
}


Shape::Shape(const vector<Primitive> &primitives, int flags) :  flags(flags), primitives(make_shared<vector<Primitive>>(primitives)) {
	calcMinMax();
}


const Shape Shape::createText(const char *text, int font) {
	return HersheyGenerator::createText(text);
}

const Shape Shape::createLetter(int code) {
	return HersheyGenerator::createLetter(code);
}

const Shape Shape::createRectangle(float w, float h, int flags) {

	vector<vec2> points;

	points.push_back(vec2(-w/2,-h/2));
	points.push_back(vec2(w/2,-h/2));
	points.push_back(vec2(w/2,h/2));
	points.push_back(vec2(-w/2,h/2));

	return Shape(points, flags | Shape::FLAG_ISLOOP | Shape::FLAG_ISBOX);
}

const Shape Shape::createCircle(float radius, int n, int flags) {
	vector<vec2> points(n);
	int i = 0;
	for(vec2 &v : points) {
		v = vec2(glm::cos(i * M_PI * 2 / n), glm::sin(i * M_PI * 2 / n)) * radius;
		i++;
	}

	return Shape(points, flags | Shape::FLAG_ISLOOP | Shape::FLAG_ISCIRCLE);
}

const Shape Shape::fromSequence(const char *seq,  float stepSize, float angleStep) {

	StepSequence ss = StepSequence(seq, stepSize, angleStep);
	return ss.getShape();
}

void Shape::getCollisionOutline(vector<vec2> &out) const {
	out = (*primitives)[0].getPoints();
}

bool Shape::contains(const vec2 &check, float radius) const {

	for(const auto &p : *primitives) {
		bool b = p.contains(check, radius);
		if(b) return true;
	}
	return false;
}


const Shape Shape::scale(double sx, double sy) const {
	const vec2 scalep(sx, sy);
	return transform([&](const vec2 &p) -> vec2 { 
		return p * scalep;
	});
}

Shape Shape::transform(std::function< glm::vec2(const glm::vec2 &point) > txFunc) const {

	auto newPrims = make_shared<vector<Primitive>>();

	for(auto &p : *primitives) {
		auto newPoints = make_shared<vector<vec2>>();
		for(const auto &v : p.getPoints()) {
			newPoints->push_back(txFunc(v));
		}
		newPrims->push_back(Primitive(newPoints, p.getFlags(), p.getColor()));
	}
	return Shape(newPrims, flags);
}

const Shape Shape::translate(const vec2 &v) const {
	return transform([&](const vec2 &p) -> vec2 { 
		return p+v;
	});
}

const Shape Shape::rotate(float angle) const {
	shared_ptr<vector<vec2>> newPoints = make_shared<vector<vec2>>();

	float r = angle * M_PI / 180;
	mat2 m;
	m[0][0] = m[1][1] = cos(r);
	m[0][1] = sin(r);
	m[1][0] = -m[0][1];

	return transform([&](const vec2 &p) -> vec2 { 
		return p*m;
	});
}

const Shape Shape::recenter(const vec2 &centerPoint) const {
	return transform([&](const vec2 &p) -> vec2 { 
		return p - centerPoint;
	});
}


const Shape Shape::concat(const Shape &shape) const {

	auto newPrims = make_shared<vector<Primitive>>(*primitives);
	//newPrims->assign(primitives);
	newPrims->insert(newPrims->end(), shape.primitives->begin(), shape.primitives->end());
	return Shape(newPrims, flags);
}


void Shape::render(const RenderContext &context) const {
	for(const Primitive &p : *primitives) {
		p.render(context);
	}
}

void Shape::render(unsigned int w, unsigned int h) const {
	for(const Primitive &p : *primitives) {
		p.render(w, h);
	}
}

const Shape Shape::roundCorners(int n, float len) const {

	auto newPrims = make_shared<vector<Primitive>>();
	for(auto &p : *primitives) {
		newPrims->push_back(p.roundCorners(n, len));
	}
	return Shape(newPrims, flags);
}

const Shape Shape::makeSolid() const {
	auto newPrims = make_shared<vector<Primitive>>();
	for(auto &p : *primitives) {
		newPrims->push_back(p.makeSolid());
	}
	return Shape(newPrims, flags);
}

const Shape Shape::setColor(const vec4 &color) const {
	auto newPrims = make_shared<vector<Primitive>>();
	for(auto &p : *primitives) {
		newPrims->push_back(p.setColor(color));
	}
	return Shape(newPrims, flags);
}

void Shape::calcMinMax() {

	min = vec2(9999999999, 9999999999);
	max = vec2(-9999999999, -9999999999);

	for(const auto &p : *primitives) {
		vec2 lo = p.getMin();
		vec2 hi = p.getMax();

		if(lo.x < min.x)
			min.x = lo.x;
		if(hi.x > max.x)
			max.x = hi.x;
		if(lo.y < min.y)
			min.y = lo.y;
		if(hi.y > max.y)
			max.y = hi.y;
	}
}


} /* namespace flatland */

#ifdef UNIT_TEST

#include "catch.hpp"

TEST_CASE("Shape", "Shape functions:") {

	using namespace flatland;
	Shape shape;
	REQUIRE(shape.size() == 0);

	shape = Shape::createRectangle(10,10);
	REQUIRE(shape.size() == 1);

}

#endif

