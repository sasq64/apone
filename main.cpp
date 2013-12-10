
#include "ModPlugin.h"
#include "ChipPlayer.h"

#include <grappix.h>
#include <SDL/SDL.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

using namespace utils;
using namespace std;

static const string pSineShader = R"(
#ifdef GL_ES
	precision mediump float;
#endif
	uniform sampler2D sTexture;
	uniform float sinepos;

	const vec4 color0 = vec4(0.0, 1.0, 0.0, 1.0);
	const vec4 color1 = vec4(1.0, 0.3, 0.3, 1.0);

	varying vec2 UV;

	void main() {
		float uvy = UV.y * 1.8 - 0.2 - sin(gl_FragCoord.x / 350.0 + sinepos) * 0.5;
		float f = gl_FragCoord.y / 400.0;
		vec4 rgb = mix(color0, color1, f);
		gl_FragColor = rgb * texture2D(sTexture, vec2(UV.x, uvy));
	}
)";

static const int bufSize = 65536;

struct App {

	Texture sprite;
	vec2f xy;
	int xpos;
	Texture scr;
	Program program;
	float sinepos;
	ModPlugin *modPlugin;
	ChipPlayer *player;

	App() : sprite {64, 64}, xy {0, 0}, xpos {-9999}, scr {screen.width()+200, 400}, sinepos {0}, player(nullptr) {

		modPlugin = new ModPlugin();
		player = modPlugin->fromFile("data/test.mod");

	    // Open the audio device, forcing the desired format
		SDL_AudioSpec wanted = { 44100, AUDIO_S16, 2, 0, bufSize/2, 0, 0, fill_audio, this };
	    if(SDL_OpenAudio(&wanted, NULL) < 0 ) {
	        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	        exit(0);
	    }
		SDL_PauseAudio(0);

		// Create our ball image
		float radius = sprite.width() / 2;
		vec2f center { sprite.width() / 2.0f, sprite.height() / 2.0f };

		sprite.clear();
		sprite.circle(center, radius, 0x000020); // Outline
		sprite.circle(center, radius*0.90, 0x0000C0); // Main ball
		sprite.circle(center + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight

		program = get_program(TEXTURED_PROGRAM).clone();
		program.setFragmentSource(pSineShader);
	}

	void update() {
		int count = 500;
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
			//v[i] = (sin(xy2 += {0.156 * 0.3, 0.187 * 0.3}) + 1.0f) * scale;
			screen.draw(sprite, (sin(xy2 += {0.126 * 0.5, 0.079 * 0.5}) + 1.0f) * scale);	
		//screen.draw_texture(sprite.id(), &v[0][0], count, sprite.width(), sprite.height(), nullptr, -1);

		program.use();
		program.setUniform("sinepos", sinepos += 0.073);

		screen.draw_texture(scr.id(), 0.0f, 0.0f, screen.width(), screen.height(), nullptr, program.id());
		screen.flip();
	}

	static void onLoad(void *arg, const char *name);
	static void onError(void *arg, int code);
	static void fill_audio(void *udata, Uint8 *stream, int len);
};


void App::fill_audio(void *udata, Uint8 *stream, int len) {	
	static vector<int16_t> buffer(bufSize);
	App *app = static_cast<App*>(udata);
	int rc = 0;
	if(app->player) {
		rc = app->player->getSamples(&buffer[0], len/2);
		memcpy(stream, &buffer[0], rc*2);
	} else {
		memset(stream, 0, len);
	}
}

void runMainLoop() {
	static App app;
	app.update();
}

int main() {
	screen.open(800, 600, false);
	LOGD("Screen is open");
	screen.renderLoop(runMainLoop);
	return 0;
}