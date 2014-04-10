#ifndef PHYSICAL_H_
#define PHYSICAL_H_

namespace flatland {

class Movable {
public:
	virtual ~Movable() {}
	//virtual void setRotation(float a) = 0;
	//virtual void setPosition(const glm::vec2 &v) = 0;
	//virtual const glm::vec2 getPosition() const = 0;;
	//virtual const float getRotation() const = 0;;
	virtual glm::vec2& scale() = 0;
	virtual float& rotation() = 0;
	virtual glm::vec2& position() = 0;
};

} /* namespace flatland */
#endif /* PHYSICAL_H_ */
