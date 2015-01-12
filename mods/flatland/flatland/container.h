#ifndef CONTAINER_H
#define CONTAINER_H

#include "node.h"

namespace flatland {

// T must implement render(RenderContext) and contains(Vec2, radius);
template <typename T> class Container : public Node {
public:
	Container(const T &shape, int flags = 0) : flags(0), currentFrame(0) {
		shapes.push_back(shape);
	}	
	Container(const std::vector<T> &shapes, int flags = 0) : flags(flags), shapes(shapes), currentFrame(0) {		
	}
	Container() : flags(0), currentFrame(0) {		
	}
	
	void renderThis(const RenderContext &context) const override {

		if((int)shapes.size() > currentFrame) {
			const T &shape = shapes[currentFrame];
			shape.render(context);
		}
	}

	bool thisContains(const glm::vec2 &check, float radius = 0) const override {

		if((int)shapes.size() > currentFrame) {
			const T &shape = shapes[currentFrame];
			return shape.contains(check, radius);
		}
		return false;
	}

	//virtual int numFrames() const override { return shapes.size(); }
	//virtual void setFrame(int f) override { currentFrame = f; }
	//virtual int getFrame() const override { return currentFrame; }

private:

	int flags;

	std::vector<T> shapes;
	int currentFrame;
	//T &shape;

};

}

#endif /* CONTAINER_H */
