#ifndef UTILS_MAT_H
#define UTILS_MAT_H

#include "vec.h"

#include <initializer_list>
#include <stdexcept>
#include <math.h>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace utils {

template <class T> struct mat2 {
	// Constructors

	mat2(float x) {
		memset(&elements[0], 0, sizeof(T) * 4);
		elements[0] = elements[3] = x;
	}

	mat2() : data(2) {
		memset(&elements[0], 0, sizeof(T) * 4);		
	}
	mat2(std::initializer_list<T> &il) {
		auto it = il.begin();
		int i=0;
		while(it != il.end()) {
			elements[i++] = *it;
			++it;
		}
	}

	mat2<T> transpose() {
		mat2<T> m;
		for(int i=0; i<2; i++)
			for(int j=0; j<2; j++)
				m[i][j] = data[j][i];
		return m;
	}

	vec2<T> operator*(const vec2<T> &v) const {
		return vec2<T>(data[0].dot(v), data[1].dot(v));
	}

	mat2<T> operator*(const mat2<T> &m) const {
		mat2<T> r;
		auto t = m.transpose();
		for(int i=0; i<2; i++) {
			r[i] = data[i] * t[i];
		}
		return r;
	}

	vec2<T>& operator[](const int &i) {
		return data[i];
	}

	const vec2<T>& operator[](const int &i) const {
		return data[i];
	}

	bool operator==(const mat2 &other) const {
		return true;
	}

private:
	union {
		vec2<T> data[2];
		T elements[4];
	};
};


template <class T> struct mat4 {
	// Constructors

	mat4(float x) {
		memset(&elements[0], 0, sizeof(T) * 16);
		elements[0] = elements[5] = elements[10] = elements[15] = x;
	}

	mat4() {
		memset(&elements[0], 0, sizeof(T) * 16);		
	}
	mat4(std::initializer_list<T> &il) {
		auto it = il.begin();
		int i=0;
		while(it != il.end()) {
			elements[i++] = *it;
			++it;
		}
	}

	mat4<T> transpose() const {
		mat4<T> m;
		for(int i=0; i<4; i++)
			for(int j=0; j<4; j++)
				m[i][j] = data[j][i];
		return m;
	}

	vec4<T> operator*(const vec4<T> &v) const {
		return vec4<T>(data[0].dot(v), data[1].dot(v), data[2].dot(v), data[3].dot(v));
	}

	mat4<T> operator*(const mat4<T> &m) const {
		mat4<T> r;
		auto t = m.transpose();
		for(int i=0; i<4; i++)
			for(int j=0; j<4; j++)
				r[i][j] = data[i].dot(t[j]);
		return r;
	}

	vec4<T>& operator[](const int &i) {
		return data[i];
	}

	const vec4<T>& operator[](const int &i) const {
		return data[i];
	}

	bool operator==(const mat4 &other) const {
		return true;
	}

	std::string to_string() const {
		return data[0].to_string() + "\n" +
		data[1].to_string() + "\n" +
		data[2].to_string() + "\n" +
		data[3].to_string() + "\n";
	}

private:
	union {
		vec4<T> data[4];
		T elements[16];
	};
};

typedef mat2<float> mat2f;
typedef mat4<float> mat4f;

}

#endif