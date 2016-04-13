#ifndef GRAPPIX_SHADER_H
#define GRAPPIX_SHADER_H

#include "color.h"
//#include <coreutils/log.h>
#include <coreutils/mat.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace grappix {

using GLuint = uint32_t;
using GLenum = uint32_t;
using GLint = int32_t;
using GLboolean = unsigned char;
using GLvoid = void;
using GLsizei = int;

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

struct ProgRef {
	ProgRef(GLint id, GLint vs, GLint fs) : id(id), vs(vs), fs(fs) {}
	~ProgRef();
	GLint id;
	GLint vs;
	GLint fs;
};

class Program {
public:
	Program(const std::string &vertexSource, const std::string &fragmentSource) : vSource(vertexSource), fSource(fragmentSource)  {
		createProgram();
	}

	Program()  {}

	Program(unsigned char *vertexSource, int vlen, unsigned char *fragmentSource, int flen) : vSource((const char*)vertexSource, vlen), fSource((const char*)fragmentSource, flen) {
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
	}

	//bool operator==(const Program &p) {
	//	return p.program == program;
	//}

	Program clone() const {
		return Program(vSource, fSource);
	}

	void createProgram();

	void setFragmentSource(const std::string &source);
	std::string getFragmentSource();
	void setVertexSource(const std::string &source);
	std::string getVertexSource();
	GLuint getAttribLocation(const std::string &name) const;
	GLuint getUniformLocation(const std::string &name) const;
	void setUniform(const std::string &name, const utils::mat4f &m) const;
	void setUniform(const std::string &name, float f0) const;
	void setUniform(const std::string &name, float f0, float f1) const;
	void setUniform(const std::string &name, float f0, float f1, float f2) const;
	void setUniform(const std::string &name, float f0, float f1, float f2, float f3) const;
	void setUniform(const std::string &name, const Color &c) const;
	void setUniform(const std::string &name, float *ptr, int count) const;
	void vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) const;
	void vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLint offset) const;
	void use() const;

	GLint id() const { return program->id; }

	static uint32_t frame_counter;
	
	operator bool() const { return program != nullptr; }

	//std::unordered_map<uint32_t, GLint> programs;
	//std::vector<std::string> flags;

//private:
	mutable std::unordered_map<std::string, GLuint> uniforms;
	mutable std::unordered_map<std::string, GLuint> attributes;
	std::string vSource;
	std::string fSource;

	//GLint program;

	std::shared_ptr<ProgRef> program;

	//GLint vertexShader;
	//GLint pixelShader;
};

const Program& get_program(program_name program);

//Program get_program_obj(program_name program);
}

#endif // GRAPPIX_SHADER_Hh