#include "MusicPlayer.h"
#include <ModPlugin/ModPlugin.h>
#include <audioplayer/audioplayer.h>
#include <grappix/grappix.h>

using namespace grappix;
using namespace utils;
using namespace std;

static const string sineShaderF = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	uniform sampler2D sTexture;
	uniform float sinepos;

	const vec4 color0 = vec4(0.0, 1.0, 0.0, 1.0);
	const vec4 color1 = vec4(1.0, 0.3, 0.3, 1.0);

	varying vec2 UV;

	void main() {
		float uvy = UV.y * 1.8 - 0.25 - sin(gl_FragCoord.x / 250.0 + sinepos) * 0.5;
		float f = gl_FragCoord.y / 400.0;
		vec4 rgb = mix(color0, color1, f);
		gl_FragColor = rgb * texture2D(sTexture, vec2(UV.x, uvy));
	}
)";

int main(int argc, char **argv) {

	screen.open(800, 600, false);

	uint32_t sz = screen.height() / 8;
	Texture sprite { sz, sz };
	vec2f xy {0,0};
	int xpos = -9999;
	Texture scr {screen.width()+200, 400};
	Program program;
	float sinepos = 0;

	auto player = MusicPlayer::fromFile("data/mods/stardust memories.mod");
	AudioPlayer aPlayer([=](int16_t *target, int len) mutable {
		player.getSamples(target, len);
	});

	Font font = Font("data/ObelixPro.ttf", 24, Font::UPPER_CASE);

	// Create our ball image
	float radius = sprite.width() / 2;
	vec2f center { sprite.width() / 2.0f, sprite.height() / 2.0f };
	sprite.clear();
	sprite.circle(center, radius, 0x000020); // Outline
	sprite.circle(center, radius*0.90, 0x0000C0); // Main ball
	sprite.circle(center + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

	program = get_program(TEXTURED_PROGRAM).clone();
	program.setFragmentSource(sineShaderF);

	screen.render_loop([=](uint32_t delta) mutable {
		int count = sz*2;
		static std::vector<vec2f> v(count);
		auto scale = vec2f(screen.size()) / 2.2;

		// Balls
		screen.clear();
		vec2f xy2 = xy += {0.001f * 0.6f * delta, 0.003f * 0.7f * delta};
		float uvs[] = { 0,0,1,0,0,1,1,1 };
		for(int i=0; i<count; i++)
			v[i] = (sin(xy2 += {0.078, 0.093}) + 1.0f) * scale;
		screen.draw_texture(sprite.id(), &v[0][0], count, sprite.width(), sprite.height());


		// Scroller
		if(sinepos > 2*M_PI)
			sinepos -= 2*M_PI;
		if(xpos < -3600)
			xpos = screen.width() + 200;
		scr.clear();
		scr.text(font, "BALLS ON THE SCREEN!!", xpos-=4, -40, 0xe080c0ff, 15.0);
		program.use();
		program.setUniform("sinepos", sinepos += (0.00373 * delta));
		screen.draw(scr, 0.0f, 0.0f, screen.width(), screen.height(), uvs, program);
		screen.flip();
	});

	screen.on_focus_lost([=]() mutable {
		aPlayer.pause();
	});

	screen.on_focus([=]() mutable {
		aPlayer.resume();
	});

	return 0;
}