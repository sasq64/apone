
#include "GL_Header.h"
#include "container.h"

using namespace glm;
using namespace std;

namespace flatland {
}

#ifdef UNIT_TEST

#include "catch.hpp"
#include "Shape.h"

TEST_CASE("Container.cpp", "Node/Container functions:") {


	using namespace flatland;
	Node *node = new Node();
	REQUIRE(node->getChildren().size() == 0);
	Shape shape = Shape::createRectangle(12,12);

	auto *c = new Container<Shape>(shape);
	c->setPosition(100,100);
	node->add(c);
	//REQUIRE(node->contains(vec2(105,105)) == true);
	//REQUIRE(node->contains(vec2(5,5)) == false);
	//node->remove(c);
	//REQUIRE(node->contains(vec2(105,105)) == false);

}

#endif


