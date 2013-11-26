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


struct App {

	TileSet font;
	bitmap space;
	SpriteLayer text;
	TileSet tiles;
	TileLayer layer;
	vec2i scrollTarget {0,0};

	void print(const std::string &t) {
		int sx = 50;
		int x = sx;
		int y = 150;
		for(auto &c : t) {
			int tile = 0;
			if(c == ' ')
				tile = 52;
			else if(c == '\n') {
				x = sx;
				y += 24*3;
				continue;
			} else
				tile = toupper(c) - 'A';
			if(tile < 0) tile = 0;
			if(tile >= 53) tile = 52;
			text.addSprite(tile, x, y, 3.0);
			x += 16 * 3;			
		}
		text.foreach([](Sprite &s) {
			auto x = (s.x - 400) * 8 + 400;
			auto y = (s.y - 200) * 12 + 200;
			float d = (rand()%30)/30.0f;
			tween::Tween::from(2.0, { { tween::delay, d }, { s.x, x }, { s.y, y } });
			tween::Tween::from(2.0, { { tween::delay, d + 1.0 }, { tween::ease, tween::easeOutBack },  { s.scale, 0.5 } });
		});
	}


	App() : font(16,24), space(16,24), text(font), tiles(16,16), layer(128, 128, 800, 450, tiles) {

		font.add_tiles(load_png(DATA_DIR "font.png"));
		
		space.clear();
		font.add_tiles(space);
	
		auto bm = load_png(DATA_DIR "tiles.png");
		LOGD("PNG LOADED");

		tiles.add_tiles(bm);

		//TileLayer layer(128, 128, 800, 450, tiles);
		for(int i=0; i<128*128; i++) {
			layer[i] = 65;
		}

		for(int i=0; i<1000; i++) {
			layer[rand()%(128*128)] = 67;
			layer[rand()%(128*128)] = 64;
		}

		LOGD("SPRITE");
		TileSet avatars(24, 32);
		avatars.add_tiles(load_png(DATA_DIR "rpgtiles.png"));
		SpriteLayer sprites(avatars);
		sprites.addSprite(0, 100, 100, 4.0);
		sprites.addSprite(1, 150, 100, 4.0);

		print("HELLO EVERYONE\nTIME TO PARTY");

		//tween::Tween::to( 3.0, { { layer.scrollx, 600 }, { layer.scrolly, 3400 } });

		
	}

	void update() {

		screen.clear();

		if(screen.key_pressed(window::RIGHT)) {
			scrollTarget.x += 16;
			tween::to(0.5, { { layer.scrollx, scrollTarget.x }, { layer.scrolly, scrollTarget.y } } );
		}
		layer.render(screen);
		text.render(screen);
		screen.flip();
	}
};

void runMainLoop() {
	static App app;
	app.update();
}

int main() {
	screen.open(640, 450, false);
	LOGD("Screen open");
	screen.renderLoop(runMainLoop);
	return 0;
}