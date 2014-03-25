#ifndef NODE_H
#define NODE_H

#include <vector>
#include <iterator>
#include <functional>

#include <glm/glm.hpp>

//#include "Renderable.h"
//#include "container.h"
#include "movable.h"
#include "rendercontext.h"
//#include "TagFunctions.h"
#include "colors.h"
//#include "Animatable.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


namespace flatland {

class Node : public Movable {
public:

	Node() : inverseDirty(false), color(Colors::GREEN), _rotation(0), oldrot(0), flags(0) {}


	virtual void add(Node *node);

	//template <typename T> void add(const T &t) {
	//	add(new Container<T>(t));
	//}

	virtual bool remove(Node *node, bool deep = false);

	virtual const std::vector<Node*> &getChildren() const { return nodes; }

	// Movable
	//virtual void setRotation(float a) override;
	//virtual void setPosition(const glm::vec2 &v) override;
	virtual float& rotation() override{ return _rotation; };
	virtual glm::vec2& position() override { return _position; };

	//const glm::vec2 getPosition() const override { return glm::vec2(matrix[2].x, matrix[2].y); }
	//const float getRotation() const override { return rotation; }

	void update();

	// Renderable
	virtual void render(const RenderContext &context);

	void print() const;

	virtual void collide(std::vector<Node*> &result, const glm::vec2 &check, float radius = 0);

	void setColor(const glm::vec4 &col) { color = col; }
	//void setPosition(float x, float y) { setPosition(glm::vec2(x,y)); };

	void setCollidable(bool on);
    void forAll(std::function<void(Node *node)> nodeFunc);
    void forAllLeaves(std::function<void(Node *node)> nodeFunc);

    const glm::mat3 &getMatrix() const { return matrix; }
    const glm::mat3 &getInvMatrix() const { 
		if(inverseDirty) {
			inverseMatrix = glm::inverse(matrix);
			inverseDirty = false;
		}
    	return inverseMatrix;
    }

    void lock(bool locked) {
    	if(locked)
    		flags |= FLAG_LOCKED;
    	else
    		flags &= (~FLAG_LOCKED);
     }

protected:

	static const int FLAG_NOCOLLISION = 256;
	static const int FLAG_LOCKED = 128;

	// Check if this node (not including children) contains the circle
	virtual bool thisContains(const glm::vec2 &check, float radius = 0) const {
		return false;
	}

	virtual void renderThis(const RenderContext &context) const {};

	std::vector<Node*> nodes;

	mutable glm::mat3 inverseMatrix;
	mutable bool inverseDirty;
	mutable glm::mat3 savedMatrix;

	glm::mat3 matrix;
	glm::vec4 color;

	float _rotation;
	float oldrot;
	glm::vec2 _position;
	glm::vec2 oldpos;

	int flags;
};

} // namespace flatland

#endif /* NODE_H */
