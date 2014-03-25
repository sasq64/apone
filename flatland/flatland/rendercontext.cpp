#include <stdio.h>

#include "GL_Header.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glutil.h"

using namespace glm;
using namespace glutil;

#include "rendercontext.h"

namespace flatland {


static const char *vShader = R"(
attribute vec4 vPosition;
uniform mat4 vMatrix;
void main() {
	gl_Position = vPosition * vMatrix;
}
)";

static const char *pShader = R"(

//precision mediump float;
uniform vec4 fColor;
//uniform sampler2D sTexture;
//in vec2 UV;

void main() {	
	gl_FragColor = fColor;	
	//gl_FragColor = texture(sTexture, UV).rgb;
}

)";




static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        fprintf(stderr, "after %s() glError (0x%x)\n", op, error);
    }
}


RenderContext::RenderContext(unsigned int w, unsigned int h) : color(glm::vec4(1,1,0,1)), viewMatrix(1.0), useColor(false), program(0) {

	float pixelsPerMM = 1.0;
	float sx = pixelsPerMM * 2 / w;
	float sy = pixelsPerMM * 2 / h;
	glm::mat4 matrix(1.0);
	glm::mat4 screenMatrix = glm::scale(matrix, glm::vec3(sx, sy, 1.0)); // 3. Scale to viewport / ratio
	viewMatrix = glutil::convert(screenMatrix);
	reloadShaders();
}

void RenderContext::setTarget(unsigned int w, unsigned int h, int buffer) {
	float pixelsPerMM = 1.0;
	float sx = pixelsPerMM * 2 / w;
	float sy = pixelsPerMM * 2 / h;
	glm::mat4 matrix(1.0);
	glm::mat4 screenMatrix = glm::scale(matrix, glm::vec3(sx, sy, 1.0)); // 3. Scale to viewport / ratio
	viewMatrix = glutil::convert(screenMatrix);

	this->buffer = buffer;
}

void RenderContext::reloadShaders() {
	printf("Reloading shaders");
	if(program) {
		glDeleteProgram(program);
	}

	printf("%s\n%s", vShader, pShader);
	program = createProgram(vShader, pShader);
	checkGlError("createProgram");

}

}


