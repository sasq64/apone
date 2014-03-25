
#include "stepsequence.h"
#include <ctype.h>
#include <string>
#include <vector>
#include <glm/gtx/spline.hpp>

using namespace glm;
using namespace std;

namespace flatland {

StepSequence::StepSequence(const char *seq, float stepSize, float angleStep) {
	double x = 0;
	double y = 0;
	double currentAngle = 0;

	//bool isLoop = false;

	int value = 0;
	bool valueSet = false;

	//float lastAngle = -1;
	double lastXstep = -1;
	double lastYstep = -1;

	int stack[32];
	int *stackptr = stack;
	const char *repeat = nullptr;
	const char *decimalPoint = nullptr;
	int repeatCount = 0;

	highp_vec2 vecValue;
	bool vectorSet = false;
	bool negative = false;

	int tags = 0;

	for(const char *sptr = seq; *sptr; sptr++) {
		char c = *sptr;

		if(isdigit(c) || c == '.' || c == '-') {
			switch(c) {
			case '-':
				if(!valueSet)
					negative = true;
				break;
			case '.':
				decimalPoint = sptr;
				break;
			default:
				value *= 10;
				value += (c - '0');
				break;
			}
			valueSet = true;
			printf("Value now %d\n", value);
		} else if(c >= 'A' && c <= 'D') {
			tags |= (1<<(c - 'A'));
		} else if(c >= 'a' && c <= 'd') {
			tags &= ~(1<<(c - 'a'));
		} else {
			float fvalue = value;
			if(valueSet) {
				if(decimalPoint) {
					fvalue *= 10;
					while(decimalPoint < sptr) {
						fvalue /= 10;
						decimalPoint++;
					}
				}
				if(negative)
					fvalue = -fvalue;
				printf("Floatvalue %f\n", fvalue);
			}


			double step = stepSize;
			//float angle = angleStep;
			double xstep = 0;
			double ystep = 0;

			if(vectorSet) {
				vecValue.y = fvalue;
				printf("Vector set to %f:%f\n", vecValue.x, vecValue.y);
				vectorSet = false;
			} else if(valueSet) {
				step = fvalue * stepSize;
				//angle = fvalue;
			}


			bool doStep = false;
			switch(c) {
			case ':':
				vectorSet = true;
				vecValue.x = fvalue;
				break;
			case '(':
				*stackptr++ = value;
				repeat = sptr;
				value = 0;
				valueSet = false;
				repeatCount = 0;
				break;
			case ')':
				if(repeatCount == 0) {
					--stackptr;
					repeatCount = *stackptr;
					sptr = repeat;
				} else {
					repeatCount--;
					if(repeatCount != 0)
						sptr = repeat;
					else
						printf("LOOP ENDS");
				}
				break;
			case 'v':
				xstep = vecValue.x * stepSize;
				ystep = vecValue.y * stepSize;
				doStep = true;
				break;
			case 'e':
				//angle = 0;
				xstep = step; ystep = 0;
				doStep = true;
				//x += step;
				break;
			case 'w':
				//angle = 180;
				xstep = -step; ystep = 0;
				doStep = true;
				//x -= step;
				break;
			case 's':
				//y += step;
				//angle = 270;
				xstep = 0; ystep = -step;
				doStep = true;
				break;
			case 'n':
				//y -= step;
				//angle = 90;
				xstep = 0; ystep = step;
				doStep = true;
				break;
			case 'f':
				xstep = cos(currentAngle*M_PI/180) * step;
				ystep = sin(currentAngle*M_PI/180) * step;
				//x = x + cos(currentAngle*M_PI/180) * step;
				//y = y + sin(currentAngle*M_PI/180) * step;
				//angle = currentAngle;
				doStep = true;
				break;
			case 'b':
				xstep = -cos(currentAngle*M_PI/180) * step;
				ystep = -sin(currentAngle*M_PI/180) * step;
				//angle = currentAngle + 180;
				//xstep = -xstep;
				//ystep = -ystep;
				doStep = true;
				//x = x - cos(currentAngle*M_PI/360) * step;
				//y = y - sin(currentAngle*M_PI/360) * step;
				break;
			case 'l':
				if(valueSet)
					currentAngle = (currentAngle - fvalue);
				else
					currentAngle = (currentAngle - angleStep);
				break;
			case 'r':
				if(valueSet)
					currentAngle = (currentAngle + fvalue);
				else
					currentAngle = (currentAngle + angleStep);
				break;
			}

			if(currentAngle > 360) currentAngle -= 360;
			if(currentAngle < 0) currentAngle += 360;


			if(doStep) {
				if(lastXstep != xstep || lastYstep != ystep) {
					// Stepping in another angle from last time, so add a new point
					//Point p = Point(x,y);
					highp_vec2 v = highp_vec2(x,y);
					printf("POINT %c %d %d (%d:%d)\n", c, (int)x, (int)y, (int)xstep, (int)ystep);
					stepPoints.push_back(StepPoint(v, tags));
				}
				lastXstep = xstep;
				lastYstep = ystep;

				x = x + xstep;
				y = y + ystep;

			}

			valueSet = false;
			negative = false;
			decimalPoint = nullptr;

			value = 0;
		}
    }

	highp_vec2 v = highp_vec2(x,y);
	printf("LASTPOINT %d %d vs FIRSTPOINT %d %d\n", (int)x, (int)y, (int)stepPoints[0].point.x, (int)stepPoints[0].point.y);

	isLoop = true;
	if(glm::distance(v, stepPoints[0].point) > 0.5) {
		stepPoints.push_back(StepPoint(v, tags));
		isLoop = false;
	}

}

void StepSequence::smooth(const char *tags, int n, float ratio) {

	vector<StepPoint> points;
	int sz = stepPoints.size();
	for(int i=0;i<sz; i++) {
		const StepPoint &sp = stepPoints[i];
		if(sp.tags & 1) {
			for(int j=0;j<=4; j++) {
				//vec2 np = catmullRom(points[(i+sz-1)%sz], points[i], points[(i+1)%sz], points[(i+2)%sz], (float)j/4.0);
				float t = (float)j/4.0;
				float t1 = (1-t);
				float t2 = 2*(1-t);

				const highp_vec2 &p0 = stepPoints[(i+sz-1)%sz].point;
				const highp_vec2 &p1 = stepPoints[i].point;
				const highp_vec2 &p2 = stepPoints[(i+1)%sz].point;

				float l0 = length(p0 - p1);
				float l1 = length(p2 - p1);

				//float half = 0.5;
				float a = ratio / l0;
				float b = ratio / l1;//1 - a;

				printf("%f %f %f\n", a,b,t);

				//const vec2 &pa = p0 + (p1 - p0) * half;
				//const vec2 &pb = p1 + (p2 - p1) * half;
				const highp_vec2 &pa = p0 * a + p1 * (1-a);
				const highp_vec2 &pb = p2 * b + p1 * (1-b);

				highp_vec2 np = pa*t1*t1 + p1*t2*t + pb*t*t;
				printf("NEWPOINT %d %d\n", (int)np.x, (int)np.y);
				points.push_back(StepPoint(np, sp.tags));
				//points2.push_back(p0);
				//points2.push_back(pa);
				//points2.push_back(pb);
				//printf("NEWPOINT %d %d\n", (int)pa.x, (int)pa.y);
				//printf("NEWPOINT %d %d\n", (int)pb.x, (int)pb.y);
			}
		} else {
			points.push_back(sp);
		}

	}
	//points = points2;

	stepPoints = points;
}

const Shape StepSequence::getShape() const {

	vector<vec2> points;
	for(auto &sp : stepPoints) {
		points.push_back(vec2(sp.point.x, sp.point.y));
	}

	return Shape(points, isLoop ? Shape::FLAG_ISLOOP : 0);
}


/*
bool StepSequence::collidePoint(const vec2 &check) {

	int nvert = points.size();
	int i, j, c = 0;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((points[i].y > check.y) != (points[j].y > check.y)) &&
		 (check.x < (points[j].x - points[i].x) * (check.y - points[i].y) / (points[j].y - points[i].y) + points[i].x))
			c++;
	}
	return (c % 2) == 1;
} */

}
