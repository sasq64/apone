#include <coreutils/log.h>
#include "tiles.h"
#include "shader.h"

#include <cmath>
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

		//LOGD("Adding tile to %d %d %dx%d", x, y, tilew, tileh);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, tilew, tileh, GL_RGBA, GL_UNSIGNED_BYTE, bmTile.flipped());
		x += tilew;
		if(x+tilew > texture.width()) {
			x = 0;
			y += tileh;
			if(y + tileh > texture.height())
				throw tile_exception("Texture full");
		}
	}

	return 0;
}

void TileSet::render_tile(int tileno, RenderTarget &target, float x, float y, double s) {
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


TileLayer::TileLayer(int w, int h, int pw, int ph, TileSet &ts) : 
	scrollx(0), scrolly(0), scale(1.0), tileset(ts), _width(w), _height(h), pixelWidth(pw), pixelHeight(ph), map(w*h) {}

void TileLayer::render(RenderTarget &target, float x0, float y0) {

	float s = 2.0;
	int tw = tileset.tilew*s;
	int th = tileset.tileh*s;

	//int pixw = pixelWidth;
	//int pixh = pixelHeight;

	int areaw = pixelWidth / tw + 1;
	int areah = pixelHeight / th + 1;

	int count = areaw*areah;

	if(multiBuf[0] == -1) {
		glGenBuffers(2, (GLuint*)multiBuf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);
		vector<uint16_t> indexes;//(count*6);
		int i=0;
		for(int j=0; j<count; j++) {
			indexes.push_back(i);
			indexes.push_back(i+1);
			indexes.push_back(i+2);
			indexes.push_back(i+1);
			indexes.push_back(i+3);
			indexes.push_back(i+2);
			i += 4;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * 2, &indexes[0], GL_STATIC_DRAW);

		vector<float> coords;//(count*16);
		coords.reserve(count*16);

		int x = 0;
		int y = 0;
		for(int iy=0; iy<areah; iy++) {
			for(int ix=0; ix<areaw; ix++) {
				coords.push_back(x);
				coords.push_back(y+th);
				coords.push_back(x+tw);
				coords.push_back(y+th);
				coords.push_back(x);
				coords.push_back(y);
				coords.push_back(x+tw);
				coords.push_back(y);
				x += tw;
			}
			y += th;
			x = 0;
		}

		glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);
		glBufferData(GL_ARRAY_BUFFER, coords.size() * 4 * 2, &coords[0], GL_DYNAMIC_DRAW);
	} else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);

	vector<float> uvs;//(count*16);
	uvs.reserve(count*8);

	int sx = scrollx / tileset.tilew;
	int sy = scrolly / tileset.tileh;
	float xx = fmod(scrollx, tileset.tilew);
	float yy = fmod(scrolly, tileset.tileh);
	if(xx < 0) xx += tileset.tilew;
	if(yy < 0) yy += tileset.tileh;

	double pw = 1.0f/tileset.texture.width();
	double ph = 1.0f/tileset.texture.height();

	double fs = pw/(s*s);//pw/10000.0;
	double ft = ph/(s*s);//pw/10000.0;

	for(int iy=0; iy<areah; iy++) {
		for(int ix=0; ix<areaw; ix++) {
			int xp = (ix+sx) % _width;
			if(xp < 0) xp += _width;
			int yp = (iy+sy) % _height;
			if(yp < 0) yp += _height;
			auto tileno = map[xp+yp*_width];
			auto tx = tileno % tileset.widthInTiles;
			auto ty = tileno / tileset.widthInTiles;

			double s0 = tx*pw*tileset.tilew + fs;
			double t0 = ty*ph*tileset.tileh + ft;
			double s1 = s0 + pw*tileset.tilew - fs*2;
			double t1 = t0 + ph*tileset.tileh - ft*2;

			uvs.push_back(s0);
			uvs.push_back(t0);
			uvs.push_back(s1);
			uvs.push_back(t0);
			uvs.push_back(s0);
			uvs.push_back(t1);
			uvs.push_back(s1);
			uvs.push_back(t1);			
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);
	glBufferSubData(GL_ARRAY_BUFFER, count*8*4, uvs.size() * 4, &uvs[0]);

	auto &program = get_program(TEXTURED_PROGRAM);
	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, target.buffer());
	glViewport(0,0,target.width(), target.height());
	glBindTexture(GL_TEXTURE_2D, tileset.texture.id());

	program.setUniform("vScreenScale", 2.0 / target.width(), 2.0 / target.height(), 0, 1);
	program.setUniform("vScale", 1.0, 1.0, 0, 1);
	program.setUniform("vPosition", -xx * s, -yy * s, 0, 1);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 0, 0);
	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, count*8*4);

	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	//glDisableVertexAttribArray(uvHandle);
	//glDisableVertexAttribArray(posHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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