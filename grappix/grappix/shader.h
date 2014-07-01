#ifndef GRAPPIX_SHADER_H
#define GRAPPIX_SHADER_H

#include "GL_Header.h"
#include "color.h"
//#include <coreutils/log.h>
#include <coreutils/mat.h>
#include <memory>
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

struct ProgRef {
	ProgRef(GLint id, GLint vs, GLint fs) : id(id), vs(vs), fs(fs) {}
	~ProgRef() {
		if(id > 0)
			glDeleteProgram(id);
		if(vs > 0)
			glDeleteShader(vs);
		if(fs > 0)
			glDeleteShader(fs);

	}
	GLint id;
	GLint vs;
	GLint fs;
};

class Program {
public:
	Program(const std::string &vertexSource, const std::string &fragmentSource) : vSource(vertexSource), fSource(fragmentSource)  {
		createProgram();
	}

	//Program(GLint p) : program(std::make_shared<ProgRef>(p)), vertexShader(-1), pixelShader(-1) {}

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

	void setFragmentSource(const std::string &source) {
		fSource = source;
		createProgram();
	}

	std::string getFragmentSource() {
		return fSource;
	}

	GLuint getAttribLocation(const std::string &name) const {
		GLuint a;
		if(attributes.count(name) == 0) {
			a = glGetAttribLocation(program->id, name.c_str());
			attributes[name] = a;
		} else {
			a = attributes[name];
		}
		return a;
	}
	GLuint getUniformLocation(const std::string &name) const {
		GLuint u;
		if(uniforms.count(name) == 0) {
			u = glGetUniformLocation(program->id, name.c_str());
			uniforms[name] = u;
		} else {
			u = uniforms[name];
		}
		return u;
	}

	void setUniform(const std::string &name, const utils::mat4f &m) const {
		auto h = getUniformLocation(name);
		glUniformMatrix4fv(h, 1, GL_FALSE, &m[0][0]);
	}

	void setUniform(const std::string &name, float f0) const {
		auto h = getUniformLocation(name);
		glUniform1f(h, f0);
	}

	void setUniform(const std::string &name, float f0, float f1) const {
		auto h = getUniformLocation(name);
		glUniform2f(h, f0, f1);
	}

	void setUniform(const std::string &name, float f0, float f1, float f2) const {
		auto h = getUniformLocation(name);
		glUniform3f(h, f0, f1, f2);
	}

	void setUniform(const std::string &name, float f0, float f1, float f2, float f3) const {
		auto h = getUniformLocation(name);
		glUniform4f(h, f0, f1, f2, f3);
	}

	void setUniform(const std::string &name, const Color &c) const {
		auto h = getUniformLocation(name);
		glUniform4f(h, c.r, c.g, c.b, c.a);
	}

	//template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
	//void setUniform(const std::string &name, T t) const {
	//	glUniform1fv();
//	}

	void vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) const {
		GLuint h = getAttribLocation(name);
		glVertexAttribPointer(h, size, type, normalized, stride, ptr);
		glEnableVertexAttribArray(h);
	}

	void vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLint offset) const {
		GLuint h = getAttribLocation(name);
		glVertexAttribPointer(h, size, type, normalized, stride, reinterpret_cast<const GLvoid*>(offset));
		glEnableVertexAttribArray(h);
	}

	void use() const { 
		glUseProgram(program->id);
		setUniform("frame_counter", frame_counter);
	}

	GLint id() const { return program->id; }

	static uint32_t frame_counter;

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