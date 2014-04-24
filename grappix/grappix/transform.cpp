
#include "transform.h"

using namespace utils;

mat4f make_rotate_z(float r) {
	mat4f matrix(1.0);
	float ca = cos(r * M_PI / 180);
	float sa = sin(r * M_PI / 180);
	matrix[0][0] = ca;
	matrix[1][1] = ca;
	matrix[0][1] = sa;
	matrix[1][0] = -sa;
	return matrix;
}

mat4f make_rotate_x(float r) {
	mat4f matrix(1.0);
	float ca = cos(r * M_PI / 180);
	float sa = sin(r * M_PI / 180);
	matrix[1][1] = ca;
	matrix[2][2] = ca;
	matrix[1][2] = sa;
	matrix[2][1] = -sa;
	return matrix;
}

mat4f make_rotate_y(float r) {
	mat4f matrix(1.0);
	float ca = cos(r * M_PI / 180);
	float sa = sin(r * M_PI / 180);
	matrix[0][0] = ca;
	matrix[2][2] = ca;
	matrix[0][2] = sa;
	matrix[2][0] = -sa;
	return matrix;
}

mat4f make_scale(float x, float y) {
	mat4f matrix(1.0);
	matrix[0][0] = x;
	matrix[1][1] = y;
	return matrix;
}

mat4f make_translate(float x, float y, float z) {
	mat4f matrix(1.0);
	matrix[0][3] = x;
	matrix[1][3] = y;
	matrix[2][3] = z;
	return matrix;
}

mat4f make_perspective(float fov, float aspect, float nearDist, float farDist) {

	mat4f result(1.0);

	if(fov <= 0 || aspect == 0) {
		return result;
	}

	float frustumDepth = farDist - nearDist;
	float oneOverDepth = 1 / frustumDepth;

	result[1][1] = 1 / tan(0.5f * fov);
	result[0][0] = result[1][1] / aspect;
	result[2][2] = farDist * oneOverDepth;
	result[3][2] = (-farDist * nearDist) * oneOverDepth;
	result[2][3] = 1;
	result[3][3] = 0;

	return result;
}