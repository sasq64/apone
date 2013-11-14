#ifndef UTILS_VEC_H
#define UTILS_VEC_H

#include <initializer_list>
#include <stdexcept>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//#include "utils/format.h"

namespace utils {

typedef unsigned int uint;

template <class T> struct vec2 {
	// Constructors

	vec2() : x(0), y(0) {}
	vec2(T x, T y) : x(x), y(y) {}
	vec2(std::pair<T, T> pair) : x(pair.first), y(pair.second) {}
	vec2(std::initializer_list<T> &il) {
		auto it = il.begin();
		x = *it;
		++it;
		y = *it;
	}

	vec2 operator+(const vec2 &v) const {
		return vec2(x + v.x, y + v.y);
	}

	vec2 operator+(const T &i) const {
		return vec2(x + i, y + i);
	}

	vec2 operator-(const vec2 &v) const {
		return vec2(x - v.x, y - v.y);
	}

	vec2 operator-(const T &i) const {
		return vec2(x - i, y - i);
	}

	vec2 operator*(const vec2 &v) const {
		return vec2(x * v.x, y * v.y);
	}

	vec2 operator*(T n) const {
		return vec2(x * n, y * n);
	}

	template <typename S> vec2 operator/(const vec2<S> &v) const {
		return vec2(x / v.x, y / v.y);
	}


	vec2 operator/(T n) const {
		return vec2(x / n, y / n);
	}

	vec2 operator+=(const vec2 &v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	vec2 operator+=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		x += xa;
		y += ya;
		return *this;
	}

	vec2 operator-=(const vec2 &v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	vec2 operator-=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		x -= xa;
		y -= ya;
		return *this;
	}

	T operator[](const int &i) const {
		return data[i];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	}

	T area() {
		return x * y;
	}

	T angle() {
		T l = sqrt(x * x + y * y);
		T a = acos(x / l);
		if(y < 0) a = (M_PI - a) + M_PI;
			return a;
	}

	T dist2() {
		return x * x + y * y;
	}

	T dist() {
		return sqrt(x * x + y * y);
	}

	union
	{
		T data[2]; /**< All components at once     */
		struct {
	        T x;   /**< Alias for first component  */
	        T y;   /**< Alias for second component */
	    };
		struct {
	        T s;   /**< Alias for first component  */
	        T t;   /**< Alias for second component */
	    };
	};
};

typedef vec2<float> vec2f;
typedef vec2<int> vec2i;

template <typename T> vec2<T> cossin(const vec2<T> &v) {
	return vec2<T>(cos(v.x), sin(v.y));
}

template <typename T> vec2<T> sin(const vec2<T> &v) {
	return vec2<T>(sinf(v.x), sinf(v.y));
}


template <class T> struct vec3 {
	// Constructors

	vec3() : x(0), y(0), z(0) {}
	vec3(T x, T y, T z) : x(x), y(y), z(z) {}
	vec3(std::initializer_list<T> &il) {
		auto it = il.begin();
		x = *it;
		++it;
		y = *it;
		++it;
		z = *it;
	}

	vec3 operator+(const vec3 &v) const {
		return vec3(x + v.x, y + v.y, z + v.z);
	}

	vec3 operator+(const T &i) const {
		return vec3(x + i, y + i, z + i);
	}

	vec3 operator-(const vec3 &v) const {
		return vec3(x - v.x, y - v.y, z - v.z);
	}

	vec3 operator-(const T &i) const {
		return vec3(x - i, y - i, z - i);
	}

	vec3 operator*(const vec3 &v) const {
		return vec3(x * v.x, y * v.y, z * v.z);
	}

	vec3 operator*(T n) const {
		return vec3(x * n, y * n, z * n);
	}

	vec3 operator/(const vec3 &v) const {
		return vec3(x / v.x, y / v.y, z / v.z);
	}

	vec3 operator/(T n) const {
		return vec3(x / n, y / n, z / n);
	}

	vec3 operator+=(const vec3 &v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	vec3 operator+=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		++it;
		auto za = *it;
		x += xa;
		y += ya;
		z += za;
		return *this;
	}

	vec3 operator-=(const vec3 &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	vec3 operator-=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		++it;
		auto za = *it;
		x -= xa;
		y -= ya;
		z -= za;
		return *this;
	}

	T operator[](const int &i) {
		return data[i];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	}


	T angle() {
		T l = sqrt(x * x + y * y + z * z);
		T a = acos(x / l);
		if(y < 0) a = (M_PI - a) + M_PI;
			return a;
	}

	T dist2() {
		return x * x + y * y + z * z;
	}

	T dist() {
		return sqrt(x * x + y * y + z * z);
	}

	union
	{
		float data[3]; /**< All components at once     */
		struct {
	        float x;   /**< Alias for first component  */
	        float y;   /**< Alias for second component */
	        float z;
	    };
		struct {
	        float s;   /**< Alias for first component  */
	        float t;   /**< Alias for second component */
	        float u;
	    };
		struct {
	        float r;   /**< Alias for first component  */
	        float g;   /**< Alias for second component */
	        float b;
	    };
	};
};

typedef vec3<float> vec3f;
typedef vec3<int> vec3i;


template <typename T> vec3<T> sin(const vec3<T> &v) {
	return vec3<T>(sinf(v.x), sinf(v.y), sin(v.z));
}




}

#endif // UTILS_VEC_H