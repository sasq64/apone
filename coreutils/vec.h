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

	bool operator==(const vec2 &other) const {
		return other.x == x && other.y == y;
	}

	bool operator!=(const vec2 &other) const {
		return other.x != x || other.y != y;
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

	vec2 operator/(const vec2 &v) const {
		return vec2(x / v.x, y / v.y);
	}

	vec2 operator*(T n) const {
		return vec2(x * n, y * n);
	}

	T dot(const vec2 &v) const {
		return x * v.x + y * v.y;
	}

	//template <typename S> vec2 operator/(const vec2<S> &v) const {
	//	return vec2(x / v.x, y / v.y);
	//}


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

	template <typename U> operator vec2<U>() {
		return vec2<U>(static_cast<U>(x), static_cast<U>(y));
	};

	T operator[](const int &i) const {
		return data[i];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	}

	T& operator[](const int &i) {
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
		T data[2];
		struct {
			T x;
			T y;
		};
		struct {
			T s;
			T t;
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
		float data[3];
		struct {
			float x;
			float y;
			float z;
		};
		struct {
			float s;
			float t;
			float u;
		};
		struct {
			float r;
			float g;
			float b;
		};
	};
};


template <typename T> vec3<T> sin(const vec3<T> &v) {
	return vec3<T>(sinf(v.x), sinf(v.y), sin(v.z));
}

typedef vec3<float> vec3f;
typedef vec3<int> vec3i;

template <class T> struct vec4 {
	// Constructors

	vec4() : x(0), y(0), z(0), w(0) {}
	vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
	vec4(std::initializer_list<T> &il) {
		auto it = il.begin();
		x = *it;
		++it;
		y = *it;
		++it;
		z = *it;
		++it;
		w = *it;
	}

	vec4 operator+(const vec4 &v) const {
		return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
	}

	vec4 operator+(const T &i) const {
		return vec4(x + i, y + i, z + i, w + i);
	}

	vec4 operator-(const vec4 &v) const {
		return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
	}

	vec4 operator-(const T &i) const {
		return vec4(x - i, y - i, z - i, w - i);
	}

	vec4 operator*(const vec4 &v) const {
		return vec4(x * v.x, y * v.y, z * v.z, w * v.w);
	}

	T dot(const vec4 &v) const {
		return x * v.x + y * v.y + z * v.z + w * v.w;
	}

	vec4 operator*(T n) const {
		return vec4(x * n, y * n, z * n, w * n);
	}

	vec4 operator/(const vec4 &v) const {
		return vec4(x / v.x, y / v.y, z / v.z, w / v.w);
	}

	vec4 operator/(T n) const {
		return vec4(x / n, y / n, z / n, w / n);
	}

	vec4 operator+=(const vec4 &v) {
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}

	vec4 operator+=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		++it;
		auto za = *it;
		++it;
		auto wa = *it;
		x += xa;
		y += ya;
		z += za;
		w += wa;
		return *this;
	}

	vec4 operator-=(const vec4 &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
		w -= v.w;
		return *this;
	}

	vec4 operator-=(std::initializer_list<T> il) {
		auto it = il.begin();
		auto xa = *it;
		++it;
		auto ya = *it;
		++it;
		auto za = *it;
		++it;
		auto wa = *it;
		x -= xa;
		y -= ya;
		z -= za;
		w -= wa;
		return *this;
	}

	T& operator[](const int &i) {
		return data[i];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	}

	const T& operator[](const int &i) const {
		return data[i];
		//throw std::out_of_range("Only 0 or 1 are valid indexes");
	}

	union
	{
		float data[4];
		struct {
			float x;  
			float y;  
			float z;
			float w;
		};
		struct {
			float s;  
			float t;  
			float u;
			float q;
		};
		struct {
			float r;  
			float g;  
			float b;
			float a;
		};
	};
};

typedef vec4<float> vec4f;
typedef vec4<int> vec4i;

//template <typename U, typename V> vec2<U> make_vec(U x, V y) {
//	return vec2<U>(x, y);
//}

template <typename T = float> struct rect {

	rect() {}
	rect(T x0, T y0, T x1, T y1) : x0(x0), y0(y0), x1(x1), y1(y1) {}

	T& operator[](const int &i) {
		return data[i];
	}

	union {
		T data[4];
		struct {
			vec2<T> p0;
			vec2<T> p1;
		};
		struct {
			T x0;
			T y0;
			T x1;
			T y1;
		};
	};
};

typedef rect<float> rectf;
typedef rect<int> recti;

}

#endif // UTILS_VEC_H