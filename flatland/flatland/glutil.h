#ifndef GLUTIL_H
#define GLUTIL_H

#include <glm/glm.hpp>
#include "GL_Header.h"

namespace glutil {
	GLuint loadShader(GLenum shaderType, const char* pSource);
	GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);
	char *readTextFile(const char *path);
	void *loadTexture(const char *path, int *w, int *h);
	void checkGlError(const char* op);

	glm::mat3 convert(const glm::mat4 &m);
	glm::mat4 convert(const glm::mat3 &m);
	glm::mat3 mcreate(float rotation, const glm::vec2 &pos);
	void mcreate(glm::mat3 &matrix, float rotation, const glm::vec2 &pos);
	inline glm::vec2 transform(const glm::mat3 &m, const glm::vec2 &v) {
		glm::vec3 v3 = m * glm::vec3(v, 1.0);
		return glm::vec2(v3.x, v3.y);
	}

}

#endif
