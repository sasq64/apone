#include <coreutils/log.h>
#include "tiles.h"

#include <vector>

using namespace std;
//using namespace utils;

//static double s = 2.0;

TileSet::TileSet() {}

TileSet::TileSet(int tilew, int tileh) : tilew(tilew), tileh(tileh), texture(256, 256) {
	widthInTiles = 256 / tilew;
	heightInTiles = 256 / tileh;

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}

int TileSet::add_tiles(const bitmap &bm) {

	int xs = 0;
	int ys = 0;
	int x = 0;
	int y = 0;

	glBindTexture(GL_TEXTURE_2D, texture.id());

	while(true) {

		auto bmTile = bm.cut(xs, ys, tilew, tileh);
		xs += tilew;
		if(xs+tilew > bm.width()) {
			xs = 0;
			ys += tileh;
			if(ys + tileh > bm.height())
				break;
		}

		LOGD("Adding tile to %d %d %dx%d", x, y, tilew, tileh);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tilew, tileh, GL_RGBA, GL_UNSIGNED_BYTE, bmTile.flipped());
		x += tilew;
		if(x+tilew > texture.width()) {
			x = 0;
			y += tileh;
			if(y + tileh > texture.height()) // TEXTURE FULL!
				break;
		}
	}

	//this->tilew = tilew;
	//this->tileh = tileh;
	//tileset = TileSet(bm, tilew, tileh);
	//texture = texture(bm);
	//widthInTiles = bm.width() / tilew;
	//heightInTiles = bm.height() / tileh;
	//this->tilew = tilew;
	//this->tileh = tileh;
	return 0;
}

void TileSet::render_tile(int tileno, basic_buffer &target, float x, float y, double s) {
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
	uvs.push_back(t0);
	uvs.push_back(s1);
	uvs.push_back(t0);
	uvs.push_back(s0);
	uvs.push_back(t1);
	uvs.push_back(s1);
	uvs.push_back(t1);
	
	target.draw_texture(texture.id(), x, y, stw, sth, &uvs[0]);
}


TileLayer::TileLayer(int w, int h, int pw, int ph, TileSet &ts) : scrollx(0), scrolly(0), tileset(ts), _width(w), _height(h), pixelWidth(pw), pixelHeight(ph), map(w*h) {}

void TileLayer::render(basic_buffer &target) {
	float s = 4.0;
	//int s = 1;

	int tilew = tileset.tilew*s;
	int tileh = tileset.tileh*s;

	int sx = scrollx / tilew;
	int sy = scrolly / tileh;
	int xx = -(scrollx % tilew);
	int yy = -(scrolly % tileh);

	for(int y=sy; y<_height; y++) {
		for(int x=sx; x<_width; x++) {
			auto tileno = map[x+_width*y];
			tileset.render_tile(tileno, target, xx, yy, s);
			//LOGD("Tile %d at %d,%d", tileno, xx, yy);
			xx += tilew;
			if(xx > pixelWidth)
				break;
		}
		yy += tileh;
		xx = -(scrollx % tilew);
		if(yy > pixelHeight)
			break;
	}
}
/*
int TileLayer::add_tiles(bitmap &bm, int tilew, int tileh) {
	this->tilew = tilew;
	this->tileh = tileh;
	tileset = TileSet(bm, tilew, tileh);
	//texture = texture(bm);
	//widthInTiles = bm.width() / tilew;
	//heightInTiles = bm.height() / tileh;
	//this->tilew = tilew;
	//this->tileh = tileh;
	return 0;
}
*/