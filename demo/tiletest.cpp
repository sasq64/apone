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


int main() {
	
	screen.open(640, 450, false);
	LOGD("Screen open");

	TileSet font(16,24);
	font.add_tiles(load_png(DATA_DIR "font.png"));
	bitmap space(16,24);
	space.clear();
	font.add_tiles(space);
	SpriteLayer text(font);

	auto print = [&](const std::string &t) {
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
	};


	auto bm = load_png(DATA_DIR "tiles.png");
	LOGD("PNG LOADED");

	TileSet tiles(16,16);
	tiles.add_tiles(bm);

	TileLayer layer(128, 128, 800, 450, tiles);
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

	vec2i scrollTarget {0,0};

	while(screen.is_open()) {
		screen.clear();
		//tiles.render(screen);
		//avatars.render_tile(0, screen, s.x, s.y, scale);
		//LOGD("Draw sprite");
		//screen.draw(0,0,sprite);

		//screen.draw_texture(tiles.texture.id(), 0, 0, 256, 256);
		////tiles.render_tile(15, screen, s.x, s.y, scale);
		//for(int i=0; i<256; i++) {
		//	tiles.render_tile(i, screen, xy[i].x, xy[i].y, scale);
		//}
		//sprites.render(screen);
/*
		auto c = screen.get_click();
		if(c.button >= 0) {
			//LOGD("%d %d", c.x, c.y);
			int x = c.x / (16*4);
			int y = c.y / (16*4);
			layer[x+y*128] = 5;
		}
*/

		if(screen.key_pressed(window::RIGHT)) {
			scrollTarget.x += 16;
			tween::to(0.5, { { layer.scrollx, scrollTarget.x }, { layer.scrolly, scrollTarget.y } } );
		}
		layer.render(screen);

		//font.render_tile(0,screen,10,10, 4.0);
		text.render(screen);

		//screen.draw(0, 0, tx);
		//screen.text(0, 0, "BALLS ON THE SCREEN!!", 0xc080c0ff, 1.0);
		screen.flip();
		//layer.scrollx++;
	}
	return 0;
}

/*
	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	texture sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight
	//float px = 0;
	//float py = 0;
	LOGD("Loop");

	Sprite s(0, 100.0, 100.0);
	float scale = 4.0;
	tween::Tween::to( 2.0, {
		//{ tween::ease, tween::easeInOutBack },
		{s.y, 200},
		{tween::delay, 1.0},
		{s.x, 508}, 
		{ tween::onComplete, [&](){
			LOGD("hello");
			//scale = 2.0;
		}}, 
	});

	vector<vec2i> xy(256);
	for(int i=0; i<256; i++) {
		vec2i p(i%16, i/16);
		xy[i] = vec2i(p.y*64.0 + 64.0, 500.0 - p.x*64.0);
		float d = (rand() % 1000) / 500.0;

		tween::Tween::to( 2.0, { { tween::delay, d }, { xy[i].x, p.x*64.0f }, { xy[i].y, p.y*64.0f } });
	}
	//tween::Tween::to( 2.0, { { scale, 4.0 } });

	scale = 4.0;

	tween::Tween::to( 3.0, { { sprites[0].x, 600.0 }, { sprites[0].y, 400.0 } });
*/

	/*
	uint32_t colors[4] = { 0xffffff00, 0xff0000ff, 0xffff0000, 0xff00ff00 };
	bitmap bm(64, 256+128);
	for(int x=0; x<64; x++)
		for(int y=0; y<64; y++) {
			uint32_t c = colors[x/16 + (y/16)*2];
			if(x == 0 || x == 16)
				c = 0xff000000;
			else if(x == 15 || x == 31)
				c = 0xffffffff;
			bm[x + y* 64] = c; 
		}

	bm[0] = 0xff80ff80;
	bm[15*32+15] = 0xffff8080;
*/

	/*
	texture tx(bm);
	LOGD("TOILES");

	TileArea tiles(32, 32);
	tiles.add_tiles(bm, 32, 32);
	for(int l=0; l<32*32; l++)
		tiles[l] = l%100;


	LOGD("AVATARS");

	TileSet avatars(load_png(DATA_DIR "rpgtiles.png"), 24, 32);
*/
	//for(int l=0; l<32; l++)
	//	tiles[rand()%(32*32)] = 16*4;

	//tiles[0] = 16;
	//tiles[1] = 16;
	//tiles[2] = 16;
	//tiles[3] = 16;
	//tiles[50] = 16;

	/*tileset tiles(32, 32);
	tiles.add_tiles(bm, 16, 16);
 
	tiles[0] = 1;

	tileset.draw(screen);
*/
