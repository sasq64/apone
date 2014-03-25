#ifndef FLATLAND_PRIMITIVE_H
#define FLATLAND_PRIMITIVE_H

#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "rendercontext.h"
//#include "Renderable.h"
#include "colors.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef short GLshort;

namespace flatland {

typedef const std::vector<const glm::vec2> vec2vec;

class Primitive  {
public:

	Primitive();
	Primitive(const std::vector<glm::vec2> &points, int flags = 0,  const glm::vec4 &color = Colors::GREEN);
    Primitive(const std::shared_ptr<const std::vector<glm::vec2>> &points, int flags = 0, const glm::vec4 &color = Colors::GREEN);
	~Primitive() {}

	static const Primitive createRectangle(float w, float h, int flags = 0);
	static const Primitive createCircle(float r, int n = 16, int flags = 0);

    bool contains(const glm::vec2 &point, float radius = 0) const;

    void getCollisionOutline(std::vector<glm::vec2> &points) const;

    const Primitive roundCorners(int n, float ratio) const;

    const Primitive scale(double s) const { return scale(s, s); };
    const Primitive scale(double sx, double sy) const;
    const Primitive translate(const glm::vec2 &v) const;
    const Primitive translate(float x, float y) const { return translate(glm::vec2(x,y)); }
    const Primitive rotate(float angle) const;


    const Primitive recenter() const {
    	return recenter(min + (max - min) * 0.5f);
    }

    const Primitive recenter(const glm::vec2 &centerPoint) const;

    bool isEmpty() const {
    	return points->size() > 0;
    };

    const std::vector<glm::vec2>& getPoints() const { return *points; }

    const Primitive makeSolid() const {
        return Primitive(points, flags | FLAG_SOLID, color);
    }

    const Primitive setColor(const glm::vec4 &col) const {
        return Primitive(points, flags, col);
    }


    void render(const RenderContext &context) const;
    void render(unsigned int w, unsigned int h) const;

    int getFlags() const { return flags; }
    const glm::vec4 &getColor() const { return color; }
    const glm::vec2 &getMin() const { return min; }
    const glm::vec2 &getMax() const { return max; }

	static const int FLAG_ISLOOP = 1;
	static const int FLAG_SOLID = 2; // Should be renders as a solid object
	static const int FLAG_ISBOX = 4; // Can be treated as a box for collision purposes
	static const int FLAG_ISCIRCLE = 8; // Can be treated as a circle for collision purposes

private:

	bool collidePoint(const glm::vec2 &point) const;
	void calculateTriIndices() const;
	void calcMinMax();

    int flags;

    glm::vec4 color;

    glm::vec2 min;
    glm::vec2 max;

    std::shared_ptr<const std::vector<glm::vec2>> points;
    
    mutable std::shared_ptr<std::vector<GLshort>> triIndices;

};
}
#endif /* FLATLAND_PRIMITIVE_H */