#include <grappix.h>

using namespace utils;

static const char *pSineShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
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

struct App {

	texture sprite;
	vec2f xy;
	int xpos;
	texture scr;
	GLuint program;
	float tstart;

	App() : sprite {64, 64}, xy {0, 0}, xpos {-9999}, scr {screen.width()+200, 400}, tstart {0} {

		// Create our ball image
		float radius = sprite.width() / 2;
		vec2f center { sprite.width() / 2.0f, sprite.height() / 2.0f };

		sprite.clear();
		sprite.circle(center, radius, 0x000020); // Outline
		sprite.circle(center, radius*0.90, 0x0000C0); // Main ball
		sprite.circle(center + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

		program = createProgram(vSineShader, pSineShader);
		LOGD("Shader created");
	}

	void update() {
		int count = 1500;
		static std::vector<vec2f> v(count);
		auto scale = vec2f(screen.size()) / 2.2;
		if(xpos < -2400)
			xpos = screen.width() + 200;

		scr.clear();
		float zoom = 7;//(sin(xpos/235.0)+4.0)*1.5;
		scr.text(xpos-=4, 20, "BALLS ON THE SCREEN!!", 0xe080c0ff, zoom);

		screen.clear();

		vec2f xy2 = xy += {0.01, 0.03};
		for(int i=0; i<count; i++)
			v[i] = (sin(xy2 += {0.156 * 0.3, 0.187 * 0.3}) + 1.0f) * scale;
			//screen.draw(sprite, (sin(xy2 += {0.156, 0.187}) + 1.0f) * scale);	
		screen.draw_texture(sprite.id(), &v[0][0], count, sprite.width(), sprite.height(), nullptr, -1);

		glUseProgram(program);
		GLuint t = glGetUniformLocation(program, "techstart");
		glUniform1f(t, tstart += 0.073);

		screen.draw_texture(scr.id(), 0.0f, 0.0f, screen.width(), screen.height(), nullptr, program);
		screen.flip();
	}

};

void runMainLoop() {
	static App app;
	app.update();
}

int main() {
	LOGD("main");	
	screen.open(800, 600, false);
	LOGD("Screen is open");
	screen.renderLoop(runMainLoop);
	return 0;
}