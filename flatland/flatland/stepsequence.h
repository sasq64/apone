
#ifndef STEPSEQUENCE_H_
#define STEPSEQUENCE_H_

#include "shape.h"

namespace flatland {

class StepPoint {
public:
	StepPoint(glm::highp_vec2 p, int t) : point(p), tags(t) {}
	glm::highp_vec2 point;
	int tags;
};


class StepSequence  {
public:

	constexpr static const char *ALL_POINTS = "!";

	StepSequence(const char *seq, float stepSize, float angleStep = 90);

	void smooth(const char *tags, int n, float ratio);

	const Shape getShape() const;


private:
	static void setCenter(std::vector<glm::vec2> &points, const glm::vec2 &center);
	bool isLoop;
	std::vector<StepPoint> stepPoints;

};

}

#endif /* STEPSEQUENCE_H_ */
