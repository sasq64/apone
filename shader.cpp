#include "shader.h"

#include <vector>

using namespace std;

static const char *vShader = R"(
	attribute vec2 vertex;
	uniform vec2 vScreenScale;
	uniform vec2 vScale;
	uniform vec2 vPosition;
	uniform float rotation;
	void main() {
		vec2 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
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
	//precision mediump float;
	uniform vec4 fColor;
	uniform sampler2D sTexture;
	varying vec2 UV;

	void main() {	
		gl_FragColor = vec4(fColor.rgb, texture2D(sTexture, UV).a);
	}
)";

static const char *vFontShader = R"(
	attribute vec2 vertex;
	attribute vec2 vUV;

	uniform vec2 vPosition;
	uniform vec2 vScale;
	uniform vec2 vScreenScale;

	varying vec2 UV;

	void main() {
		vec2 v = vertex * vScale + vPosition;
		gl_Position = vec4(v.x * vScreenScale.x - 1.0, 1.0 - v.y * vScreenScale.y, 0, 1);
		UV = vUV;
	}
)";

static const char *pDFontShader = R"(
	uniform vec4 fColor;
	uniform sampler2D sTexture;
	varying vec2 UV;

	vec3 glyph_color    = vec3(1.0,1.0,1.0);
	const float glyph_center   = 0.50;
	vec3 outline_color  = vec3(0.0,0.0,1.0);
	const float outline_center = 0.58;
	vec3 glow_color     = vec3(1.0,1.0,1.0);
	const float glow_center    = 1.25;

	void main() {
		vec4 color = texture2D(sTexture, UV);
		float dist  = color.a;
		float width = fwidth(dist);
		float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);

		//float mu = smoothstep(outline_center-width, outline_center+width, dist);
		//vec3 rgb = mix(outline_color, glyph_color, mu);
		//gl_FragColor = vec4(rgb, max(alpha,mu));

		//vec3 rgb = mix(glow_color, fColor.rgb, alpha);
		//float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
		//gl_FragColor = vec4(rgb, max(alpha,mu));
		gl_FragColor = vec4(fColor.rgb, fColor.a * alpha);

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
		programs.resize(4);
		programs[FLAT_PROGRAM] = createProgram(vShader, pShader);
		programs[TEXTURED_PROGRAM] = createProgram(vTexShader, pTexShader);
		programs[FONT_PROGRAM] = createProgram(vFontShader, pFontShader);
		programs[FONT_PROGRAM_DF] = createProgram(vFontShader, pDFontShader);
	}
	return programs[program];
}

