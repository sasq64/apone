#ifndef GRAPPIX_SHADER_H
#define GRAPPIX_SHADER_H

#include <GL/glew.h>
#include <stdexcept>
#include <string>

class shader_exception : public std::exception {
public:
	shader_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

GLuint loadShader(GLenum shaderType, const std::string &source);
GLuint createProgram(const std::string &vertexSource, const std::string &fragmentSource);

enum program_name {
	FLAT_PROGRAM,
	TEXTURED_PROGRAM,
	FONT_PROGRAM,
	FONT_PROGRAM_DF
};

GLuint get_program(program_name program);

#endif // GRAPPIX_SHADER_H