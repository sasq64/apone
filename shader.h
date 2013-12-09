#ifndef GRAPPIX_SHADER_H
#define GRAPPIX_SHADER_H

#include "GL_Header.h"
#include <stdexcept>
#include <string>
#include <unordered_map>

class shader_exception : public std::exception {
public:
	shader_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

GLuint loadShader(GLenum shaderType, const std::string &source);
GLuint createProgram(const std::string &vertexSource, const std::string &fragmentSource);
GLuint createProgram(unsigned char *vertexSource, int vlen, unsigned char *fragmentSource, int flen);

enum program_name {
	FLAT_PROGRAM,
	TEXTURED_PROGRAM,
	FONT_PROGRAM,
	FONT_PROGRAM_DF
};

class Program {
public:
	Program(const std::string &vertexSource, const std::string &fragmentSource) : vSource(vertexSource), fSource(fragmentSource) {
		program = createProgram(vertexSource, fragmentSource);
	}

	Program(GLuint p) : program(p) {}

	GLuint getAttribLocation(const std::string &name) {
		GLuint a;
		if(attributes.count(name) == 0) {
			a = glGetAttribLocation(program, name.c_str());
			attributes[name] = a;
		} else {
			a = attributes[name];
		}
		return a;
	}
	GLuint getUniformLocation(const std::string &name) {
		GLuint u;
		if(uniforms.count(name) == 0) {
			u = glGetUniformLocation(program, name.c_str());
			uniforms[name] = u;
		} else {
			u = uniforms[name];
		}
		return u;
	}
private:
	std::unordered_map<std::string, GLuint> uniforms;
	std::unordered_map<std::string, GLuint> attributes;
	GLuint program;
	std::string vSource;
	std::string fSource;
};

GLuint get_program(program_name program);

Program get_program_obj(program_name program);


#endif // GRAPPIX_SHADER_H