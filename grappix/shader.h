#ifndef GRAPPIX_SHADER_H
#define GRAPPIX_SHADER_H

#include "GL_Header.h"
#include <coreutils/log.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace grappix {

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
	Program(const std::string &vertexSource, const std::string &fragmentSource) : vSource(vertexSource), fSource(fragmentSource), program(-3), vertexShader(-1), pixelShader(-1) {
		createProgram();
	}

	Program(GLint p) : program(p), vertexShader(-1), pixelShader(-1) { LOGD("PPPP %d", p);}

	Program() : program(-2), vertexShader(-1), pixelShader(-1) {}

	Program(unsigned char *vertexSource, int vlen, unsigned char *fragmentSource, int flen) : vSource((const char*)vertexSource, vlen), fSource((const char*)fragmentSource, flen), program(-4), vertexShader(-1), pixelShader(-1) {
		createProgram();
	}

	/* Program& operator=(const Program &p) {
		uniforms.clear();
		attributes.clear();
		program = p.program;
		vSource = p.vSource;
		fSource = p.fSource;
		pixelShader = p.pixelShader;
		vertexShader = p.vertexShader;
		return *this;
	}

	Program(const Program &p) {
		program = p.program;
		vSource = p.vSource;
		fSource = p.fSource;
		pixelShader = p.pixelShader;
		vertexShader = p.vertexShader;
	} */

	~Program() {
		program = -5;
		LOGD("Destroy");
	}

	bool operator==(const Program &p) {
		return p.program == program;
	}

	Program clone() {
		return Program(vSource, fSource);
	}

	void createProgram();

	void setFragmentSource(const std::string &source) {
		fSource = source;
		createProgram();
	}

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

	void setUniform(const std::string &name, float f0) {
		auto h = getUniformLocation(name);
		glUniform1f(h, f0);
	}

	void setUniform(const std::string &name, float f0, float f1) {
		auto h = getUniformLocation(name);
		glUniform2f(h, f0, f1);
	}

	void setUniform(const std::string &name, float f0, float f1, float f2) {
		auto h = getUniformLocation(name);
		glUniform3f(h, f0, f1, f2);
	}

	void setUniform(const std::string &name, float f0, float f1, float f2, float f3) {
		auto h = getUniformLocation(name);
		glUniform4f(h, f0, f1, f2, f3);
	}

	void vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) {
		GLuint h = getAttribLocation(name);
		glVertexAttribPointer(h, size, type, normalized, stride, ptr);
		glEnableVertexAttribArray(h);
	}

	void vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLint offset) {
		GLuint h = getAttribLocation(name);
		glVertexAttribPointer(h, size, type, normalized, stride, reinterpret_cast<const GLvoid*>(offset));
		glEnableVertexAttribArray(h);
	}

	void use() { glUseProgram(program); }

	GLint id() { return program; }

//private:
	std::unordered_map<std::string, GLuint> uniforms;
	std::unordered_map<std::string, GLuint> attributes;
	std::string vSource;
	std::string fSource;

	GLint program;
	GLint vertexShader;
	GLint pixelShader;
};

Program& get_program(program_name program);

//Program get_program_obj(program_name program);
}

#endif // GRAPPIX_SHADER_Hh