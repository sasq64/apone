#ifndef SHAPE_H
#define SHAPE_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <functional>

#include "primitive.h"
#include "rendercontext.h"
//#include "Renderable.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef short GLshort;


namespace flatland {

typedef const std::vector<const glm::vec2> vec2vec;

class Shape  {
public:

	Shape();
	Shape(const std::vector<Primitive> &primitives, int flags = 0);
	Shape(const std::shared_ptr<const std::vector<Primitive>> &primitives, int flags = 0);

	Shape(const std::vector<glm::vec2> &points, int flags = 0);
	~Shape() {}

	static const Shape createRectangle(float w, float h, int flags = 0);
	static const Shape createCircle(float r, int n = 16, int flags = 0);
	static const Shape fromSequence(const char *seq,  float stepSize = 10, float angleStep = 90);

	static const Shape createText(const char *text, int font = 0);
	static const Shape createLetter(int code);

	Shape transform(std::function<glm::vec2(const glm::vec2 &point)> txFunc) const;

	bool contains(const glm::vec2 &point, float radius = 0) const;

	void getCollisionOutline(std::vector<glm::vec2> &points) const;

	const Shape roundCorners(int n, float ratio) const;

	const Shape scale(double s) const { return scale(s, s); };
	const Shape scale(double sx, double sy) const;
	const Shape translate(const glm::vec2 &v) const;
	const Shape translate(float x, float y) const { return translate(glm::vec2(x,y)); }
	const Shape rotate(float angle) const;


	const Shape recenter() const {
		return recenter(min + (max - min) * 0.5f);
	}

	const Shape recenter(const glm::vec2 &centerPoint) const;

	const Shape makeSolid() const;
	const Shape setColor(const glm::vec4 &color) const;

	const Shape concat(const Shape &shape) const;

	bool isEmpty() const {
		return primitives->size() > 0;
	};

	int size() const {
		return primitives->size();
	}

	//const std::vector<glm::vec2>& getPoints() const { return *points; }

	void render(const RenderContext &context) const;
	void render(unsigned int w, unsigned int h) const;

	static const int FLAG_ISLOOP = 1;
	//static const int FLAG_SOLID = 2; // Should be renders as a solid object
	static const int FLAG_ISBOX = 4; // Can be treated as a box for collision purposes
	static const int FLAG_ISCIRCLE = 8; // Can be treated as a circle for collision purposes

	const glm::vec2 &getMin() const { return min; }
	const glm::vec2 &getMax() const { return max; }

	const float getWidth() const { return max.x - min.x; }
	const float getHeight() const { return max.y - min.y; }

private:


	void calcMinMax();

	int flags;
	glm::vec2 min;
	glm::vec2 max;
	std::shared_ptr<const std::vector<Primitive>> primitives;
};

}

#endif
