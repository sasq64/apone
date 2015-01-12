
#include "node.h"
#include "shape.h"
#include "container.h"
#include <algorithm>

using namespace std;
using namespace glm;

namespace flatland {

	void Node::add(Node *node) { 
		nodes.push_back(node);
	}

	Node* Node::add(const Shape &s) {
		nodes.push_back(new Container<Shape>(s));
		return nodes.back();
	}

	Node* Node::add(const Primitive &s) {
		nodes.push_back(new Container<Primitive>(s));
		return nodes.back();
	}

	bool Node::remove(Node *node, bool deep) {
		auto it = find(nodes.begin(), nodes.end(), node);
		if(it != nodes.end()) {
			nodes.erase(it);
			return true;
		} else if(deep) {
			for(Node *n : nodes) {
				if(n->remove(node, deep))
					return true;
			}
		}
		return false;
	}

	// Movable
/*	void Node::setRotation(float a) {
		rotation = a;

		float ca = cos(a * M_PI / 180);
		float sa = sin(a * M_PI / 180);
		matrix[0][0] = matrix[1][1] = ca;
		matrix[0][1] = sa;
		matrix[1][0] = -sa;

		inverseDirty = true;
	}
*/

	void Node::update() {

		if(_rotation != oldrot || _scale != oldscale) {
	
			float ca = cos(_rotation * M_PI / 180);
			float sa = sin(_rotation * M_PI / 180);
			matrix[0][0] = ca * _scale.x;
			matrix[1][1] = ca * _scale.y;
			matrix[0][1] = sa;
			matrix[1][0] = -sa;
			inverseDirty = true;
			oldrot = _rotation;
			oldscale = _scale;
		}

		if(_position != oldpos) {
			matrix[2].x = _position.x;
			matrix[2].y = _position.y;
			inverseDirty = true;
		}

	}

	//void Node::setPosition(const vec2 &v) {
	//	matrix[2] = vec3(v,1);
	//	inverseDirty = true;
	//}

	// Renderable
	void Node::render(const RenderContext &context) {

		update();

		RenderContext localCtx = context;

		if(flags & FLAG_LOCKED) {
			localCtx.viewMatrix = savedMatrix;
		} else {
			savedMatrix = localCtx.viewMatrix;
		}
		localCtx.viewMatrix *= matrix;

		localCtx.color = color;

		renderThis(localCtx);

		for(Node *node : nodes) {
			node->render(localCtx);
		}

	}

	void Node::print() const {
		printf("%p (%d)\n", this, (int)nodes.size());
		for(Node *node : nodes) {
			node->print();
		}		
	}

	void Node::collide(vector<Node*> &result, const vec2 &check, float radius) {

		if(flags & FLAG_NOCOLLISION)
			return;

		if(inverseDirty) {
			inverseMatrix = inverse(matrix);
			inverseDirty = false;
		}

		const vec3 p = inverseMatrix * vec3(check, 1);
		const vec2 localCheck(p.x,p.y);

		result.push_back(this);
	
		if(thisContains(localCheck, radius)) {
			return;
		}

		for(auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
			Node *node = *it;
			unsigned int l = result.size();
			node->collide(result, localCheck, radius);
			if(result.size() != l) {
				return;
			}
		}
		result.pop_back();
	}

	void Node::setCollidable(bool on) {
		if(on)
			flags &= (~FLAG_NOCOLLISION);
		else
			flags |= FLAG_NOCOLLISION;
	}

    void Node::forAll(function<void(Node *node)> nodeFunc) {
    	nodeFunc(this);
    	for(Node *n : nodes) {
    		nodeFunc(n);
    		n->forAll(nodeFunc);
    	}
    }

    void Node::forAllLeaves(function<void(Node *node)> nodeFunc) {
    	if(nodes.size() > 0) {
	    	for(Node *n : nodes) {
	    		n->forAllLeaves(nodeFunc);
	    	}    		
    	} else
    		nodeFunc(this);
    }
}
