#include "GL_Header.h"
#include <vector>
#include <algorithm>
#include <ctype.h>
#include <glm/gtx/spline.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
//#include "glutil.h"
#include "ConvexDecomposition/b2Polygon.h"

#include "primitive.h"

using namespace glm;
using namespace std;
//using namespace glutil;

namespace flatland {

Primitive::Primitive() : flags(0), color(Colors::GREEN), points(make_shared<vector<vec2>>())  {
}

Primitive::Primitive(const vector<vec2> &pts, int flags, const vec4 &color) : flags(flags), color(color), points(make_shared<vector<vec2>>(pts)) {
	if(pts.size() > 0 && pts[0] == pts[pts.size()-1])
		flags |= FLAG_SOLID;
	calcMinMax();
}

Primitive::Primitive(const shared_ptr<const vector<glm::vec2>> &points, int flags, const vec4 &color) :  flags(flags), color(color), points(points) {
	if(points->size() > 0 && (*points)[0] == (*points)[points->size()-1])
		flags |= FLAG_SOLID;
	calcMinMax();
}

const Primitive Primitive::createRectangle(float w, float h, int flags) {

	vector<vec2> points;

	points.push_back(vec2(-w/2,-h/2));
	points.push_back(vec2(w/2,-h/2));
	points.push_back(vec2(w/2,h/2));
	points.push_back(vec2(-w/2,h/2));

	return Primitive(points, flags | Primitive::FLAG_ISLOOP | Primitive::FLAG_ISBOX);
}

const Primitive Primitive::createCircle(float radius, int n, int flags) {
	vector<vec2> points(n);
	int i = 0;
	for(vec2 &v : points) {
		v = vec2(glm::cos(i * M_PI * 2 / n), glm::sin(i * M_PI * 2 / n)) * radius;
		i++;
	}

	return Primitive(points, flags | Primitive::FLAG_ISLOOP | Primitive::FLAG_ISCIRCLE);
}


bool Primitive::collidePoint(const vec2 &p) const {

	if(p.x < min.x || p.y < min.y || p.x > max.x || p.y > max.y)
		return false;

	const vector<vec2> &points = *this->points;
	int nvert = points.size();
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((points[i].y > p.y) != (points[j].y > p.y)) &&
		 (p.x < (points[j].x - points[i].x) * (p.y - points[i].y) / (points[j].y - points[i].y) + points[i].x))
			c++;
	}
	return (c % 2) == 1;
}

static float inline dist2(const vec2 &a, const vec2 &b) {
	vec2 v = a - b;
	return v.x * v.x + v.y * v.y;
}

bool Primitive::contains(const vec2 &p, float radius) const {

	if(radius == 0)
		return collidePoint(p);

	float r2 = radius * radius;

	const vector<vec2> &points = *this->points;

	int nvert = points.size();
	int i, j;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		const vec2 &pA = points[j];
		const vec2 &pB = points[i];
		float l2 = dist2(pB, pA);
		if (l2 == 0.0)
			l2 = dist2(p, pA);
		else {
			float t = glm::dot(p - pA, pB - pA) / l2;
			if(t < 0.0)
				l2 = dist2(p, pA);
			else if(t > 1.0)
				l2 = dist2(p, pB);
			else {
				 vec2 projection = pA + t * (pB - pA);
				 l2 = dist2(p, projection);
			}
		}
		if(l2 < r2)
			return true;
	}
	return collidePoint(p);

}


const Primitive Primitive::scale(double sx, double sy) const {

	shared_ptr<vector<vec2>> newPoints = make_shared<vector<vec2>>();

	const vec2 sv = vec2(sx, sy);
	for(const auto &p : *points) {
		newPoints->push_back(p * sv);
	}

	return Primitive(newPoints, flags, color);
}

const Primitive Primitive::translate(const vec2 &v) const {

	shared_ptr<vector<vec2>> newPoints = make_shared<vector<vec2>>();

	for(const auto &p : *points) {
		newPoints->push_back(p + v);
	}
	return Primitive(newPoints, flags, color);
}

const Primitive Primitive::rotate(float angle) const {

	shared_ptr<vector<vec2>> newPoints = make_shared<vector<vec2>>();

	float r = angle * M_PI / 180;
	mat2 m;
	m[0][0] = m[1][1] = cos(r);
	m[0][1] = sin(r);
	m[1][0] = -m[0][1];

	for(const auto &p : *points) {
		newPoints->push_back(p * m);
	}
	return Primitive(newPoints, flags, color);
}

const Primitive Primitive::recenter(const vec2 &centerPoint) const {

	shared_ptr<vector<vec2>> newPoints = make_shared<vector<vec2>>();

	for(const vec2 &p : *points) {
		newPoints->push_back(p - centerPoint);
	}

	return Primitive(newPoints, flags, color);
}


void Primitive::render(unsigned int w, unsigned int h) const {
	static RenderContext context;
	context.setTarget(w, h);
	render(context);
}

void Primitive::render(const RenderContext &context) const {

	GLuint program = context.program;

	if(context.buffer >=0)
		glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)context.buffer);

	GLuint posHandle = glGetAttribLocation(program, "vPosition");
	GLuint matrixHandle = glGetUniformLocation(program, "vMatrix");
	GLuint colorHandle = glGetUniformLocation(program, "fColor");

	glUseProgram(program);

	// Transpose & convert to 4x4
	const mat3 &m3 = context.viewMatrix;
	mat4 m4 = mat4(1.0);

	m4[0][0] = m3[0][0];
	m4[1][0] = m3[0][1];

	m4[0][1] = m3[1][0];
	m4[1][1] = m3[1][1];

	m4[0][3] = m3[2][0];
	m4[1][3] = m3[2][1];

	glUniformMatrix4fv(matrixHandle, 1, GL_FALSE, &m4[0][0]);
	if(context.useColor)
		glUniform4fv(colorHandle, 1, &context.color[0]);
	else
		glUniform4fv(colorHandle, 1, &color[0]);

	const vector<vec2> &points = *this->points;
	glVertexAttribPointer(posHandle, 2, GL_FLOAT, GL_FALSE, 0, &points[0][0]);
	glEnableVertexAttribArray(posHandle);


	if((flags & (FLAG_SOLID|FLAG_ISLOOP)) == (FLAG_SOLID|FLAG_ISLOOP)) {
		if(!triIndices) {
			calculateTriIndices();
		}
		int sz = triIndices->size();
		glDrawElements(GL_TRIANGLES, sz, GL_UNSIGNED_SHORT, &(*triIndices)[0]);
	} else
		glDrawArrays(flags & FLAG_ISLOOP ? GL_LINE_LOOP : GL_LINE_STRIP, 0, points.size());
}

const Primitive Primitive::roundCorners(int n, float len) const {

	vector<vec2> newPoints;
	const vector<vec2> &points = *this->points;

	int sz = points.size();

	newPoints.reserve(sz);

	for(int i=0;i<sz; i++) {
		const vec2 &sp = points[i];
		if(true) {
			for(int j=0;j<=n; j++) {

				float t = (float)j/(float)n;
				float t1 = (1-t);
				float t2 = 2*(1-t);

				const vec2 &p0 = points[(i+sz-1)%sz];
				const vec2 &p1 = points[i];
				const vec2 &p2 = points[(i+1)%sz];

				float l0 = length(p0 - p1);
				float l1 = length(p2 - p1);

				float a = len / l0;
				float b = len / l1;

				const vec2 &pa = p0 * a + p1 * (1-a);
				const vec2 &pb = p2 * b + p1 * (1-b);

				vec2 np = pa*t1*t1 + p1*t2*t + pb*t*t;
				newPoints.push_back(np);
			}
		} else {
			newPoints.push_back(sp);
		}
	}

	return Primitive(newPoints, flags, color);

}

void Primitive::calcMinMax() {

	min = vec2(9999999999, 9999999999);
	max = vec2(-9999999999, -9999999999);

	for(const vec2 &v : *points) {
		if(v.x < min.x)
			min.x = v.x;
		else if(v.x > max.x)
			max.x = v.x;

		if(v.y < min.y)
			min.y = v.y;
		else if(v.y > max.y)
			max.y = v.y;
	}
}

void Primitive::calculateTriIndices() const {

	int sz = points->size();

	vector<b2Vec2> b2vecs;

	std::transform(points->begin(), points->end(), back_inserter(b2vecs), [](const vec2 &p) {
		return b2Vec2(p.x, p.y);
	});

	b2Polygon poly = b2Polygon(&b2vecs[0], b2vecs.size());

	 if(poly.IsCCW()) {
		ReversePolygon(poly.x, poly.y, poly.nVertices);
	 }

	b2Triangle *results = new b2Triangle [sz];

	int count = TriangulatePolygon(poly.x, poly.y, poly.nVertices, results);

	if(count < 1) {
		delete [] results;
	}

	int i,j;

	triIndices = make_shared<vector<GLshort>>(count * 3);
	vector<GLshort> &indices = *triIndices;

	for(i=0; i<count*3; i++) {
		const b2Triangle &t = results[i/3];
		float x = t.x[i%3];
		float y = t.y[i%3];

		for(j=0; j<sz; j++) {
			if(x == poly.x[j] && y == poly.y[j]) {
				indices[i] = j;
				break;
			}
		}
		if(j == sz) {
			printf("ERROR");
			break;
		}
	}

	delete [] results;
}


} /* namespace flatland */

#ifdef UNIT_TEST

#include "catch.hpp"

TEST_CASE("Primitive", "Primitive functions:") {

	using namespace flatland;
	Primitive primitive;
	REQUIRE(primitive.getPoints().size() == 0);

	primitive = Primitive::createRectangle(10,10);
	REQUIRE(primitive.getPoints().size() == 4);
	REQUIRE(primitive.contains(vec2(4,4)) == true);
	primitive = primitive.translate(20,20);
	REQUIRE(primitive.contains(vec2(4,4)) == false);
}

#endif

