#include <grappix/grappix.h>
#include <coreutils/vec.h>
#include <vector>

using namespace std;
using namespace utils;
using namespace grappix;

#ifdef ANDROID
#define DATA_DIR "/sdcard/"
#else
#define DATA_DIR "data/"
#endif

struct App {

	TileSet tiles;
	TileLayer layer;
	TileSet avatars;
	SpriteLayer sprites;

	vec2f scrollTarget {0,0};

	App() : tiles(16,16), layer(128, 128, 800, 480, tiles), avatars(24, 32), sprites(avatars) {
	
		auto bm = load_png(DATA_DIR "tiles.png");
		LOGD("PNG LOADED");

		tiles.add_tiles(bm);
		layer.scale = 4.0;

		//TileLayer layer(128, 128, 800, 450, tiles);
		for(int i=0; i<128*128; i++) {
			layer[i] = 65;
		}

		for(int i=0; i<1000; i++) {
			layer[rand()%(128*128)] = 67;
			layer[rand()%(128*128)] = 64;
		}

		LOGD("SPRITE");
		try {
			avatars.add_tiles(load_png(DATA_DIR "rpgtiles.png"));
		} catch (tile_exception &e) {
			LOGD("Tiles wont fit!");
		}
		sprites.addSprite(0, 100, 100, 2.0);
		sprites.addSprite(1, 150, 100, 2.0);

		//tween::Tween::to( 3.0, { { layer.scrollx, 600 }, { layer.scrolly, 3400 } });

		tween::to(1.0, { { sprites[1].x, 300 } });
		
	}


	void update() {

		screen.clear();

		if(screen.key_pressed(Window::RIGHT)) {
			scrollTarget.x += 16;
			tween::to(0.5, { { layer.scrollx, scrollTarget.x }, { layer.scrolly, scrollTarget.y } } );
		}

		auto key = screen.get_key();
		if(key >= '1' && key <= '4')
			tween::to(0.5, {{ layer.scale, (float)(key - '1') + 1.0f }});

		auto click = screen.get_click();
		if(click != Window::NO_CLICK) {
			scrollTarget.x = layer.scrollx + click.x - screen.width()/2;
			scrollTarget.y = layer.scrolly + click.y - screen.height()/2;
			tween::to(0.5, { { layer.scrollx, scrollTarget.x }, { layer.scrolly, scrollTarget.y } } );
		}

		layer.scrollx += 0.1;
		layer.scrolly += 0.1;

		layer.render(screen);
		sprites.render(screen);
		screen.text(format("x:%d", (int)layer.scrollx), 0,-10,0xffffffff, 2.0);
		screen.flip();
	}
};

void runMainLoop(uint32_t d) {
	static App app;
	app.update();
}

int main() {
	screen.open(800, 480, false);
	LOGD("Screen open");
	screen.render_loop(runMainLoop);
	return 0;
}