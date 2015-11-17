#include "shader.h"

#include <cstring>
#include <vector>
#include <coreutils/log.h>

#include "GL_Header.h"

using namespace std;

extern unsigned char _shader_plain_v_glsl[];
extern unsigned char _shader_plain_f_glsl[];
extern unsigned char _shader_texture_v_glsl[];
extern unsigned char _shader_texture_f_glsl[];
extern unsigned char _shader_font_v_glsl[];
extern unsigned char _shader_font_f_glsl[];
extern unsigned char _shader_fontdf_f_glsl[];

extern int _shader_plain_v_glsl_len;
extern int _shader_plain_f_glsl_len;
extern int _shader_texture_v_glsl_len;
extern int _shader_texture_f_glsl_len;
extern int _shader_font_v_glsl_len;
extern int _shader_font_f_glsl_len;
extern int _shader_fontdf_f_glsl_len;

namespace grappix {

uint32_t Program::frame_counter = 0;

GLuint loadShader(GLenum shaderType, const std::string &source) {
	//LOGD("Compiling shader");
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
				LOGD("Shader failed msg: %s", msg);
			}
			LOGD("Shader fail:\n%s\n", source);
			throw shader_exception(msg);
		}
	} else
		throw shader_exception("Failed to create shader");

	return shader;
}

GLuint createProgram(unsigned char *vertexSource, int vlen, unsigned char *fragmentSource, int flen) {
	char vsource[vlen+1];
	memcpy(vsource, vertexSource, vlen);
	vsource[vlen] = 0;
	char fsource[flen+1];
	memcpy(fsource, fragmentSource, flen);
	fsource[flen] = 0;
	return createProgram(vsource, fsource);
}


GLuint createProgram(const string &vertexSource, const string &fragmentSource) {
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);

#ifdef GL_ES
	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, string("\nprecision mediump float;\n") + fragmentSource);
#else
	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
#endif

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
				glGetProgramInfoLog(program, bufLength, NULL, buf);
				msg = buf;
			}
			LOGD("Link failed\n%s\n", msg);
			glDeleteProgram(program);
			throw shader_exception(msg);
		}
	} else
		throw shader_exception("Could not create program");
	return program;
}

const Program& get_program(program_name program) {
	static vector<Program> programs;
	if(programs.size() == 0) {
		//LOGD("RESIZE");
		programs.resize(4);
		programs[FLAT_PROGRAM] = Program(_shader_plain_v_glsl, _shader_plain_v_glsl_len, _shader_plain_f_glsl,_shader_plain_f_glsl_len);
		programs[TEXTURED_PROGRAM] = Program(_shader_texture_v_glsl, _shader_texture_v_glsl_len, _shader_texture_f_glsl, _shader_texture_f_glsl_len);
		programs[FONT_PROGRAM] = Program(_shader_font_v_glsl, _shader_font_v_glsl_len, _shader_font_f_glsl, _shader_font_f_glsl_len);
		programs[FONT_PROGRAM_DF] = Program(_shader_font_v_glsl, _shader_font_v_glsl_len, _shader_fontdf_f_glsl, _shader_fontdf_f_glsl_len);
	}
	//LOGD("returning %p", &programs[program]);

	return programs[program];
}


ProgRef::~ProgRef() {
	if(id > 0)
		glDeleteProgram(id);
	if(vs > 0)
		glDeleteShader(vs);
	if(fs > 0)
		glDeleteShader(fs);

}


void Program::createProgram() {

	uniforms.clear();

	//if(vertexShader >= 0)
	//	glDeleteShader(vertexShader);
	//if(pixelShader >= 0)
	//	glDeleteShader(pixelShader);

	//if(program >= 0)
	//	glDeleteProgram(program);

	GLint vertexShader = loadShader(GL_VERTEX_SHADER, vSource.c_str());

#ifdef GL_ES
	GLint pixelShader = loadShader(GL_FRAGMENT_SHADER, (string("\nprecision mediump float;\n") + fSource).c_str());
#else
	GLint pixelShader = loadShader(GL_FRAGMENT_SHADER, fSource.c_str());
#endif

	GLint p = glCreateProgram();
	if(p) {
		glAttachShader(p, vertexShader);
		glAttachShader(p, pixelShader);
		glLinkProgram(p);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(p, GL_LINK_STATUS, &linkStatus);
		if(linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			string msg;
			glGetProgramiv(p, GL_INFO_LOG_LENGTH, &bufLength);
			if(bufLength) {
				char buf[bufLength];
				glGetProgramInfoLog(p, bufLength, NULL, buf);
				msg = buf;
			}
			LOGD("Link failed\n%s\n", msg.c_str());
			glDeleteProgram(p);
			throw shader_exception(msg);
		}
		program = make_shared<ProgRef>(p, vertexShader, pixelShader);
	} else
		throw shader_exception("Could not create program");

}


void Program::setFragmentSource(const std::string &source) {
	fSource = source;
	createProgram();
}

std::string Program::getFragmentSource() {
	return fSource;
}

void Program::setVertexSource(const std::string &source) {
	vSource = source;
	createProgram();
}

std::string Program::getVertexSource() {
	return vSource;
}

GLuint Program::getUniformLocation(const std::string &name) const {
	GLuint u;
	if(uniforms.count(name) == 0) {
		u = glGetUniformLocation(program->id, name.c_str());
		uniforms[name] = u;
	} else {
		u = uniforms[name];
	}
	return u;
}

void Program::setUniform(const std::string &name, const utils::mat4f &m) const {
	auto h = getUniformLocation(name);
	glUniformMatrix4fv(h, 1, GL_FALSE, &m[0][0]);
}

void Program::setUniform(const std::string &name, float f0) const {
	auto h = getUniformLocation(name);
	glUniform1f(h, f0);
}

void Program::setUniform(const std::string &name, float f0, float f1) const {
	auto h = getUniformLocation(name);
	glUniform2f(h, f0, f1);
}

void Program::setUniform(const std::string &name, float f0, float f1, float f2) const {
	auto h = getUniformLocation(name);
	glUniform3f(h, f0, f1, f2);
}

void Program::setUniform(const std::string &name, float f0, float f1, float f2, float f3) const {
	auto h = getUniformLocation(name);
	glUniform4f(h, f0, f1, f2, f3);
}

void Program::setUniform(const std::string &name, const Color &c) const {
	auto h = getUniformLocation(name);
	glUniform4f(h, c.r, c.g, c.b, c.a);
}

void Program::setUniform(const std::string &name, float *ptr, int count) const {
	auto h = getUniformLocation(name);
	glUniform1fv(h, count, ptr);
}

//template <typename T, class = typename std::enable_if<std::is_compound<T>::value>::type>
//vProgram::oid setUniform(const std::string &name, T t) const {
//	glUniform1fv();
//	}

GLuint Program::getAttribLocation(const std::string &name) const {
	GLuint a;
	if(attributes.count(name) == 0) {
		a = glGetAttribLocation(program->id, name.c_str());
		attributes[name] = a;
	} else {
		a = attributes[name];
	}
	return a;
}
void Program::vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr) const {
	GLuint h = getAttribLocation(name);
	glVertexAttribPointer(h, size, type, normalized, stride, ptr);
	glEnableVertexAttribArray(h);
}

void Program::vertexAttribPointer(const std::string &name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLint offset) const {
	GLuint h = getAttribLocation(name);
	glVertexAttribPointer(h, size, type, normalized, stride, reinterpret_cast<const GLvoid*>(offset));
	glEnableVertexAttribArray(h);
}

void Program::use() const {
	glUseProgram(program->id);
	setUniform("frame_counter", frame_counter);
}



}
