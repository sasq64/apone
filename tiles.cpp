#include <grappix.h>
#include <vector>

using namespace std;
using namespace utils;

//static double s = 2.0;

struct TileSet {

	TileSet() {}

	TileSet(const bitmap &bm, int tilew, int tileh) : tilew(tilew), tileh(tileh), texture(bm) {
		widthInTiles = bm.width() / tilew;
		heightInTiles = bm.height() / tileh;		
	}

	void render_tile(int tileno, basic_buffer &target, float x, float y, double s) {
		auto tx = tileno % widthInTiles;
		auto ty = tileno / widthInTiles;

		double pw = 1.0f/texture.width();
		double ph = 1.0f/texture.height();

		double stw = tilew*s;
		double sth = tileh*s;

		double fs = pw/(s*s);//pw/10000.0;
		double ft = ph/(s*s);//pw/10000.0;
		double s0 = tx*pw*tilew + fs;
		double t0 = ty*ph*tileh + ft;
		double s1 = s0 + pw*tilew - fs*2;
		double t1 = t0 + ph*tileh - ft*2;

		vector<float> uvs;
		uvs.push_back(s0);
		uvs.push_back(t1);
		uvs.push_back(s1);
		uvs.push_back(t1);
		uvs.push_back(s0);
		uvs.push_back(t0);
		uvs.push_back(s1);
		uvs.push_back(t0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		
		target.draw_texture(texture.texture(), x, y, stw, sth, &uvs[0]);
	}

	int tilew;
	int tileh;
	int widthInTiles;
	int heightInTiles;
	renderbuffer texture;
};


class TileArea {
public:
	TileArea(int w, int h) : _width(w), _height(h), map(w*h), target(screen) {}

	void render() {
		float s = 4.0;
		for(int y=0; y<_height; y++) {
			for(int x=0; x<_width; x++) {
				auto tileno = map[x+_width*y];
				tileset.render_tile(tileno, target, x*tilew*s, y*tileh*s, s);
			}
		}
	}
/*
		int sx = 0;
		int sy = 0;
		int bw = texture.width();
		int bh = texture.height();

		// Size of a texel in UV space
		double pw = 1.0/bw;
		double ph = 1.0/bh;

		double stw = tilew*s;
		double sth = tileh*s;

		for(int y=0; y<_height; y++)
			for(int x=0; x<_width; x++) {
				auto tileno = map[x+_width*y];
				auto tx = tileno % widthInTiles;
				auto ty = tileno / widthInTiles;

				// tilew/tileh = 16, wit = 4
				//bmw/bmh = 32/32

				//LOGD("%d %f %d", tx, pw, tilew);
				double fs = pw/(s*s);//pw/10000.0;
				double ft = ph/(s*s);//pw/10000.0;
				double s0 = tx*pw*tilew + fs;
				double t0 = ty*ph*tileh + ft;
				double s1 = s0 + pw*tilew - fs*2;
				double t1 = t0 + ph*tileh - ft*2;
				//float t0 = ty/(float)(heightInTiles+0);
				//float s1 = (tx+1)/(float)(widthInTiles+0);
				//float t1 = (ty+1)/(float)(heightInTiles+0);
				//LOGD("%f %f %f %f", s0, t0, s1, t1);
				vector<float> uvs;
				uvs.push_back(s0);
				uvs.push_back(t1);
				uvs.push_back(s1);
				uvs.push_back(t1);
				uvs.push_back(s0);
				uvs.push_back(t0);
				uvs.push_back(s1);
				uvs.push_back(t0);
				target.draw_texture(texture.texture(), x*tilew*s + sx, y*tileh*s + sy, stw, sth, &uvs[0]);
				//target.rectangle((float)x*tilew + sx, (float)y*tileh + sy, (float)tilew, (float)tileh, 0xffff00ff);
			}
		//s *= 0.999;
	}
*/
	int add_tiles(bitmap bm, int tilew, int tileh) {
		this->tilew = tilew;
		this->tileh = tileh;
		tileset = TileSet(bm, tilew, tileh);
		//texture = renderbuffer(bm);
		//widthInTiles = bm.width() / tilew;
		//heightInTiles = bm.height() / tileh;
		//this->tilew = tilew;
		//this->tileh = tileh;
		return 0;
	}

	uint32_t &operator[](uint32_t i) {
		return map[i];
	}

private:

	TileSet tileset;

	int _width;
	int _height;
	int tilew;
	int tileh;
	vector<uint32_t> map;
	basic_buffer &target;
};

int main() {
	
	screen.open(true);


	auto bm = read_png_file("tiles.png");
/*
	uint32_t colors[4] = { 0xffffff00, 0xff0000ff, 0xffff0000, 0xff00ff00 };
	bitmap bm(32,32);
	for(int x=0; x<32; x++)
		for(int y=0; y<32; y++) {
			uint32_t c = colors[x/16 + (y/16)*2];
			if(x == 0 || x == 16)
				c = 0xff000000;
			else if(x == 15 || x == 31)
				c = 0xffffffff;
			bm[x + y* 32] = c; 
		}

	bm[0] = 0xff80ff80;
	bm[15*32+15] = 0xffff8080;
*/
	renderbuffer tx(bm);

	TileArea tiles(32, 32);
	tiles.add_tiles(bm, 16, 16);
	for(int l=0; l<32*32; l++)
		tiles[l] = l%100;


	TileSet avatars(read_png_file("rpgtiles.png"), 24, 32);

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

	// Create our ball image
	vec2f size {128, 128};
	auto radius = size[0] / 2;
	renderbuffer sprite(size);
	sprite.clear();
	sprite.circle(size/2, radius, 0x000020); // Outline
	sprite.circle(size/2, radius*0.9, 0x0000C0); // Main ball
	sprite.circle(size/2 + vec2f{radius*0.15f, radius*0.15f}, radius * 0.6, 0x0040FF); // Hilight
	float px = 0;
	float py = 0;
	while(screen.is_open()) {
		screen.clear();
		tiles.render();
		avatars.render_tile(0, screen, px++, py++, 4.0);
		//screen.draw(0, 0, tx);
		//screen.text(0, 0, "BALLS ON THE SCREEN!!", 0xc080c0ff, 36.0);
		screen.flip();
	}
	return 0;
}
