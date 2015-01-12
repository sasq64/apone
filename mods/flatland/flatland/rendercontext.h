#ifndef RENDERCONTEXT_H_
#define RENDERCONTEXT_H_

#include <glm/glm.hpp>

typedef unsigned int GLuint;

namespace flatland {

class RenderContext {
public:
	RenderContext(unsigned int w = 0, unsigned int h = 0);

	void setTarget(unsigned int w, unsigned int h, int buffer = -1);

	glm::vec4 color;

	//int attributes;
	//static const int SOLID = 1;

	glm::mat3 viewMatrix;
	//glm::mat3 invViewMatrix;
	bool useColor;
	GLuint program;
	int buffer;
private:
	//const char *vertexShader;
	//const char *pixelShader;
	void reloadShaders();
};

}

#endif /* RENDERCONTEXT_H_ */
