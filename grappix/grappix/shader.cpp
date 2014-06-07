#include "shader.h"

#include <cstring>
#include <vector>
#include <coreutils/log.h>

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
			LOGD("Link failed\n%s\n", msg.c_str());
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

void Program::createProgram() {

	uniforms.clear();

	if(vertexShader >= 0)
		glDeleteShader(vertexShader);
	if(pixelShader >= 0)
		glDeleteShader(pixelShader);

	if(program >= 0)
		glDeleteProgram(program);

	vertexShader = loadShader(GL_VERTEX_SHADER, vSource.c_str());

#ifdef GL_ES
	pixelShader = loadShader(GL_FRAGMENT_SHADER, (string("\nprecision mediump float;\n") + fSource).c_str());
#else
	pixelShader = loadShader(GL_FRAGMENT_SHADER, fSource.c_str());
#endif

	program = glCreateProgram();
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
			LOGD("Link failed\n%s\n", msg.c_str());
			glDeleteProgram(program);
			throw shader_exception(msg);
		}
	} else
		throw shader_exception("Could not create program");
}

}
