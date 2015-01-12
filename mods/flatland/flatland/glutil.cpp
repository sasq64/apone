#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "glutil.h"

using namespace glm;

namespace glutil {


mat3 mcreate(float rotation, const vec2 &pos) {

	mat3 matrix;
	float ca = cos(rotation * M_PI / 180);
	float sa = sin(rotation * M_PI / 180);
	matrix[0][0] = matrix[1][1] = ca;
	matrix[0][1] = sa;
	matrix[1][0] = -sa;
	matrix[2] = vec3(pos.x, pos.y, 1);
	return matrix;
}

void mcreate(mat3 &matrix, float rotation, const vec2 &pos) {
	
	float ca = cos(rotation * M_PI / 180);
	float sa = sin(rotation * M_PI / 180);
	matrix[0][0] = matrix[1][1] = ca;
	matrix[0][1] = sa;
	matrix[1][0] = -sa;
	matrix[2] = vec3(pos.x, pos.y, 1);
}

mat3 convert(const mat4 &m4) {
	mat3 m3 = mat3(1.0);
	m3[0][0] = m4[0][0];
	m3[0][1] = m4[0][1];
	m3[1][0] = m4[1][0];
	m3[1][1] = m4[1][1];
	m3[2][0] = m4[3][0];
	m3[2][1] = m4[3][1];
	return m3;
}

mat4 convert(const mat3 &m3) {
	mat4 m4 = mat4(1.0);
	m4[0][0] = m3[0][0];
	m4[0][1] = m3[0][1];
	m4[1][0] = m3[1][0];
	m4[1][1] = m3[1][1];
	m4[3][0] = m3[2][0];
	m4[3][1] = m3[2][1];
	return m4;
}

	
GLuint loadShader(GLenum shaderType, const char* pSource) {
	GLuint shader = glCreateShader(shaderType);
	if (shader) {
		glShaderSource(shader, 1, &pSource, NULL);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				char* buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					fprintf(stderr, "ERROR:%s\n", buf);
					free(buf);
					exit(0);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		return 0;
	}

	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program) {
		glAttachShader(program, vertexShader);
	   // checkGlError("glAttachShader");
		glAttachShader(program, pixelShader);
		//checkGlError("glAttachShader");
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			if (bufLength) {
				char* buf = (char*) malloc(bufLength);
				if (buf) {
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					free(buf);
				}
			}
			glDeleteProgram(program);
			program = 0;
		}
	}
	return program;
}

char *readTextFile(const char *path) {
	static char buffer[65536];
	char *data = nullptr;
	auto fp = fopen(path, "rb");
	if(fp) {
		int len = fread(buffer, 1, sizeof(buffer), fp);
		if(len > 0) {
			data = (char*) malloc(len+1);
			memcpy(data, buffer, len);
			data[len] = 0;
		}
		fclose(fp);
	}
	return data;
}

int get32(unsigned char *ptr) {
	return ptr[0] | (ptr[1]<<8) | (ptr[2]<<16) | (ptr[3]<<24);
}

void *loadTexture(const char *path, int *w, int *h) {
	unsigned char header[54];
	memset(header, 0, 54);
	FILE *fp = fopen(path, "rb");
	if(fp) {
		int rc = fread(header, 1, 54, fp);
		//int dataPos = get32(&header[0x0A]);
		int imageSize = get32(&header[0x22]);
		int width = get32(&header[0x12]);
		int height = get32(&header[0x16]);

		if(rc != 54 || header[0] != 'B' || width < 0 || width > 65536)
			return nullptr;

		if(imageSize == 0) {
			imageSize = width*height*3;
		}

		*w = width;
		*h = height;

		char *pixels = new char [imageSize];
		fread(pixels, 1, imageSize, fp);
		fclose(fp);
		return pixels;
	}
	return nullptr;
}


//addProgram(const char *vertexFile, const char *fragmentFile)




}
