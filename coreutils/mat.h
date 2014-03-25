#ifndef UTILS_MAT_H
#define UTILS_MAT_H

#include <initializer_list>
#include <stdexcept>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace utils {


template <class T, int size> struct matrix {
	// Constructors

	matrix() : data(size*size) {}
	matrix(std::initializer_list<T> &il) {
		auto it = il.begin();
		int i=0;
		while(it != il.end()) {
			data[i++] = *it;
			++it;
		}
	}

	//T operator[](const int &i) const {
	//	return data[i];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	//}

	T* operator[](const int &i) {
		return &data[i*size];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	}

	bool operator==(const matrix &other) const {
		//return other.x == x && other.y == y;
		return true;
	}
private:
	std::vector<T> data;
};

typedef matrix<float, 2> mat2f;
typedef matrix<float, 3> mat3f;
typedef matrix<float, 4> mat4f;

}

#endif