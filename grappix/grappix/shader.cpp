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

/*
static const char *vShader = R"(
	attribute vec2 vertex;
	uniform vec2 vScreenScale;
	uniform vec2 vScale;
	uniform vec2 vPosition;
	//uniform float rotation;
	void main() {
		vec2 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
	}
)";

static const char *pShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	uniform vec4 vColor;
	void main() {
		gl_FragColor = vColor;	
	}
)";


static const char *pTexShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	//uniform vec4 fColor;
	uniform sampler2D sTexture;
	varying vec2 UV;

	void main() {	
		gl_FragColor = texture2D(sTexture, UV);
	}
)";


static const char *vTexShader = R"(
	attribute vec4 vertex;
	attribute vec2 uv;

	varying vec2 UV;

	uniform vec4 vScreenScale;
	uniform vec4 vScale;
	uniform vec4 vPosition;

	void main() {
		vec4 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
		UV = uv;
	}
)";

static const char *pFontShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	uniform vec4 fColor;
	uniform sampler2D sTexture;
	varying vec2 UV;

	void main() {	
		gl_FragColor = vec4(fColor.rgb, texture2D(sTexture, UV).a);
	}
)";

static const char *vFontShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	attribute vec4 vertex;
	attribute vec2 uv;

	uniform vec4 vPosition;
	uniform vec4 vScale;
	uniform vec4 vScreenScale;

	varying vec2 UV;

	void main() {
		vec4 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
		UV = uv;
	}
)";

static const char *pDFontShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	uniform vec4 vColor;
	uniform vec4 vScale;
	uniform sampler2D sTexture;
	//uniform float smoothing;
	varying vec2 UV;

	vec3 glyph_color    = vec3(1.0,1.0,1.0);
	const float glyph_center   = 0.50;
	vec3 outline_color  = vec3(0.0,0.0,1.0);
	const float outline_center = 0.58;
	vec3 glow_color     = vec3(1.0,1.0,1.0);
	const float glow_center    = 1.25;

	void main() {
		float dist = texture2D(sTexture, UV).a;
#ifdef GL_ES
		float smoothing = 1.0 / (vScale.x * 16.0);
		float alpha = smoothstep(glyph_center-smoothing, glyph_center+smoothing, dist);
#else
		float width = fwidth(dist);
		float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);
#endif

		//gl_FragColor = vec4(vColor.rgb, vColor.a * alpha);
		gl_FragColor = vec4(1,1,1, alpha);

		//float mu = smoothstep(outline_center-width, outline_center+width, dist);
		//vec3 rgb = mix(outline_color, glyph_color, mu);
		//gl_FragColor = vec4(rgb, max(alpha,mu));

		//vec3 rgb = mix(glow_color, fColor.rgb, alpha);
		//float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
		//gl_FragColor = vec4(rgb, max(alpha,mu));

	}
)";
*/

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

Program& get_program(program_name program) {
	static vector<Program> programs;
	if(programs.size() == 0) {
		//LOGD("RESIZE");
		programs.resize(4);
		//programs[FLAT_PROGRAM] = createProgram(vShader, pShader);
		//programs[TEXTURED_PROGRAM] = createProgram(vTexShader, pTexShader);
		//programs[FONT_PROGRAM] = createProgram(vFontShader, pFontShader);
		//programs[FONT_PROGRAM_DF] = createProgram(vFontShader, pDFontShader);
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
	pixelShader = loadShader(GL_FRAGMENT_SHADER, fSource.c_str());

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

/*
Program get_program_obj(program_name program) {
	auto p = get_program(program);
	return Program(p);
}*/

}
