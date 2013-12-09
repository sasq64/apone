#include <grappix.h>
#include <coreutils/vec.h>
#include <vector>
using namespace std;
using namespace utils;

#ifdef ANDROID
#define DATA_DIR "/sdcard/"
#else
#define DATA_DIR "data/"
#endif

#include "tween.h"

//using namespace tween;
uint8_t *make_distance_map(uint8_t *img, int width, int height);

int main() {
	
	screen.open(640, 450, false);
	LOGD("Screen open");


	auto map = load_png(DATA_DIR "map.png");

	uint8_t data8[map.width() * map.height()];
	for(int i=0; i<map.size(); i++) {
		data8[i] = map[i] & 0xff000000 ? 0xff : 0x00;
		//if(i<5000)
		//	LOGD("%02x", data8[i]);
	}

	uint8_t *ddata = make_distance_map(data8, map.width(), map.height());

	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, map.width(), map.height(), 0, GL_ALPHA, GL_UNSIGNED_BYTE, ddata);

	//auto texture = texture(map);
	auto program = get_program(FONT_PROGRAM_DF);

static const char *vShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	uniform vec4 vColor;
	uniform vec4 vScale;
	uniform sampler2D sTexture;
	//uniform float smoothing;
	varying vec2 UV;

	vec3 glyph_color    = vec3(0.0,1.0,0.0);
	const float glyph_center   = 0.50;
	vec3 outline_color  = vec3(0.0,0.0,1.0);
	const float outline_center = 0.58;
	vec3 glow_color     = vec3(1.0, 1.0, 0.0);
	const float glow_center    = 1.0;

	void main() {
		float dist = texture2D(sTexture, UV).a;
#ifdef GL_ES
		float smoothing = 1.0 / (vScale.x * 16.0);
		float alpha = smoothstep(glyph_center-smoothing, glyph_center+smoothing, dist);
#else
		float width = fwidth(dist);
		float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);
		//float alpha = dist;
#endif

		//gl_FragColor = vec4(1.0, 0.0, 0.0, alpha);
		//vec3 rgb = mix(vec3(0,0,0), vec3(1.0,0.0,0.0), dist);
		//gl_FragColor = vec4(rgb, 1.0);//floor(dist + 0.500));
		gl_FragColor = vec4(vColor.rgb, vColor.a * alpha);
		//gl_FragColor = vec4(1.0, 0.0, 0.0, floor(dist + 0.500));
		//gl_FragColor += vec4(0.0, 1.0, 0.0, floor(dist + 0.533));

		//float mu = smoothstep(outline_center-width, outline_center+width, dist);
		//vec3 rgb = mix(outline_color, glyph_color, mu);
		//gl_FragColor = vec4(rgb, max(alpha,mu));

		//vec3 rgb = mix(glow_color, vec3(1.0,1.0,1.0), alpha);
		//float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
		//gl_FragColor = vec4(rgb, mu);//max(alpha,mu));

	}
)";

	LOGD("Program %d", program);
//	glUseProgram(program);
	float scale = 1.0;
	float y = 0;
	while(screen.is_open()) {
		screen.clear();
		screen.scale(scale*=1.01);
		screen.draw_texture(id, 0, y, map.width(), map.height(), nullptr, -1);
		//y -= (scale / 2.0);
		//if(screen.key_pressed(Window::RIGHT)) {
		//	scrollTarget.x += 16;
		//	tween::to(0.5, { { layer.scrollx, scrollTarget.x }, { layer.scrolly, scrollTarget.y } } );
		//}
		screen.flip();
	}
	return 0;
}
