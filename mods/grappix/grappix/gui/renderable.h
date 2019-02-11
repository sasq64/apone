#ifndef RENDERABLE_H
#define RENDERABLE_H

#include <grappix/grappix.h>
#include <memory>

namespace grappix {
class RenderTarget;
}

class RenderSet;

class Renderable {
public:
	virtual ~Renderable() {}

	virtual void render(std::shared_ptr<grappix::RenderTarget>, uint32_t delta) = 0;
	virtual void visible(bool b) { is_visible = b; }
	virtual bool visible() { return is_visible; }
	//virtual void setTarget(std::shared_ptr<grappix::RenderTarget> target) { this->target = target; }

	void setParent(RenderSet *p) { parent = p; }

	RenderSet *getParent() { return parent; }

	void remove();

	bool is_visible = true;
	RenderSet *parent;
	//std::shared_ptr<grappix::RenderTarget> target;
};

#endif // RENDERABLE_H
