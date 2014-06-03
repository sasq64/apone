#ifndef GRAPPIX_RECTANGLE_H
#define GRAPPIX_RECTANGLE_H

namespace grappix {

struct Rectangle {
	Rectangle() {}
	Rectangle(float w, float h) : x(0), y(0), w(w), h(h), rot(0.0) {}
	Rectangle(float x, float y, float w, float h, float rot = 0.0) : x(x), y(y), w(w), h(h), rot(rot) {}
	union {
		float p[5];
		struct {
			float x;
			float y;
			float w;
			float h;
			float rot;
		};
	};
	float& operator[](const int &index) { return p[index]; }

	Rectangle operator/(const Rectangle &r) const {
		return Rectangle(x, y, w / r.w, h / r.h);
	}

};

}

#endif // GRAPPIX_RECTANGLE_H
