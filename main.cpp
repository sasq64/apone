#include <grappix.h>

using namespace utils;

static const char *pSineShader = R"(
	//precision mediump float;
	uniform vec4 fColor;
	uniform vec2 mosaic;
	//const vec2 mosaic = vec2(100.0,100.0);
	uniform sampler2D sTexture;
	uniform float techstart;

	const vec4 color0 = vec4(0.0, 1.0, 0.0, 1.0);
	const vec4 color1 = vec4(1.0, 0.3, 0.3, 1.0);

	varying vec2 UV;

	void main() {
		//vec2 uv = vec2(UV.x - sin(gl_FragCoord.y / 10.0 + techstart) * 0.004, UV.y);

		float uvy = UV.y * 1.8 - 0.2 - sin(gl_FragCoord.x / 350.0 + techstart) * 0.5;

		float f = gl_FragCoord.y / 400.0;
		vec4 rgb = mix(color0, color1, f);

		//uv = floor(uv * mosaic) / mosaic ;//vec2(ceil(uv.x * 50.0)/50.0, ceil(uv.y * 50.0)/50.0);
		gl_FragColor = rgb * texture2D(sTexture, vec2(UV.x, uvy));
	}
)";


static const char *vSineShader = R"(
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

int main() {
	
	screen.open(true);
	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.90, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

	renderbuffer scr(screen.width()+200, 400);
	
	GLuint program = createProgram(vSineShader, pSineShader);
	
	// Loop and render ball worm
	vec2f xy{0, 0};
	auto scale = vec2f(screen.size()) / 2.3;
	int xpos = screen.width() + 400;
	float tstart = 0;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	while(screen.is_open()) {
		scr.clear();
		float zoom = 7;//(sin(xpos/235.0)+4.0)*1.5;
		scr.text((xpos-=4), 20, "BALLS ON THE SCREEN!!", 0xe080c0ff, zoom);

		screen.clear();
		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<100; i++)
			screen.draw((sin(xy2 += {0.156, 0.187}) + 1.0f) * scale, sprite);		

		glUseProgram(program);
		GLuint t = glGetUniformLocation(program, "techstart");
		glUniform1f(t, tstart+=0.04);

		screen.draw_texture(scr.texture(), 0, 0, screen.width(), screen.height(), nullptr, program);

		screen.flip();
	}
	return 0;
}
