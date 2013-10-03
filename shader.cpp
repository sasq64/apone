#include "shader.h"

#include <vector>

using namespace std;

static const char *vShader = R"(
	attribute vec4 vPosition;
	void main() {
		gl_Position = vPosition;
	}
)";

static const char *pShader = R"(
	uniform vec4 fColor;
	void main() {	
		gl_FragColor = fColor;	
	}
)";


static const char *pTexShader = R"(
	//precision mediump float;
	uniform vec4 fColor;
	uniform sampler2D sTexture;
	varying vec2 UV;

	void main() {	
		gl_FragColor = texture2D(sTexture, UV);
	}
)";

static const char *vTexShader = R"(
	attribute vec4 vPosition;
	attribute vec2 vUV;

	varying vec2 UV;

	void main() {
		gl_Position = vPosition;
		UV = vUV;
	}
)";

GLuint loadShader(GLenum shaderType, const std::string &source) {
	GLuint shader = glCreateShader(shaderType);
	if(shader) {
		const char *sources[1];
		sources[0] = source.c_str();
		glShaderSource(shader, 1, sources, NULL);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if(!compiled) {
			GLint infoLen = 0;
			string msg;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if(infoLen) {
				char buf[infoLen];
				glGetShaderInfoLog(shader, infoLen, NULL, buf);
				msg = buf;
				glDeleteShader(shader);
				shader = 0;
			}
			throw shader_exception(msg);
		}
	} else
		throw shader_exception("Failed to create shader");

	return shader;
}

GLuint createProgram(const string &vertexSource, const string &fragmentSource) {
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);

	GLuint program = glCreateProgram();
	if(program) {
		glAttachShader(program, vertexShader);
		glAttachShader(program, pixelShader);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if(linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			string msg;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			if(bufLength) {
				char buf[bufLength];
				if(buf) {
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					msg = buf;
				}
			}
			glDeleteProgram(program);
			throw shader_exception(msg);
		}
	} else
		throw shader_exception("Could not create program");
	return program;
}

GLuint get_program(program_name program) {
	static vector<GLuint> programs;
	if(programs.size() == 0) {
		programs.resize(2);
		programs[FLAT_PROGRAM] = createProgram(vShader, pShader);
		programs[TEXTURED_PROGRAM] = createProgram(vTexShader, pTexShader);
	}
	return programs[program];
}

