#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <coreutils/mat.h>
#define _USE_MATH_DEFINES
#include <cmath>

utils::mat4f make_rotate_z(float r);
utils::mat4f make_rotate_x(float r);
utils::mat4f make_rotate_y(float r);
utils::mat4f make_scale(float x, float y);
utils::mat4f make_translate(float x, float y, float z = 0.0);
utils::mat4f make_perspective(float fov, float aspect, float nearDist, float farDist);

#endif // TRANSFORM_H