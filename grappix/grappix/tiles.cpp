#include "tiles.h"
#include "shader.h"

#include <coreutils/log.h>
#include <image/packer.h>
#include <cmath>
#include <vector>

using namespace std;
using namespace image;

//using namespace utils;

//static double s = 2.0;

namespace grappix {

TileSet::TileSet() : xpos(0), ypos(0) {}

TileSet::TileSet(uint32_t tilew, uint32_t tileh, uint32_t texw, uint32_t texh) : tilew(tilew), tileh(tileh), texture(texw, texh), xpos(0), ypos(0)  {
	//widthInTiles = texw / tilew;
	//heightInTiles = texh / tileh;

	packer = make_shared<Packer>(texw, texh);

	tiles = make_shared<vector<tile>>();

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

int TileSet::add(const bitmap &bm) {

	int tilew = bm.width();
	int tileh = bm.height();

	image::Packer::Rect r(0, 0, tilew, tileh);
	packer->add(r);

	LOGD("Adding %dx%d to pos %d,%d", tilew, tileh, xpos, ypos);

	if(ypos + tileh > texture.height())
		throw tile_exception("Texture full");

	double pw = 1.0f/texture.width();
	double ph = 1.0f/texture.height();

	float s0 = xpos * pw;
	float s1 = s0 + pw*tilew;
	float t1 = 1.0 - ypos*ph;
	float t0 = t1 - ph*tileh;

	tiles->emplace_back(s0, t0, s1, t1, tilew, tileh);

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, texture.height()-tileh-ypos, tilew, tileh, GL_RGBA, GL_UNSIGNED_BYTE, bm.flipped());
	xpos += tilew;
	if(xpos+tilew > texture.width()) {
		xpos = 0;
		ypos += tileh;
	}

	return tiles->size()-1;
}

int TileSet::add_tile(int tx, int ty, int tw, int th) {

	double pw = 1.0f/texture.width();
	double ph = 1.0f/texture.height();

	float s0 = tx * pw;
	float s1 = s0 + pw*tw;
	float t1 = 1.0 - ty*ph;
	float t0 = t1 - ph*th;

	tiles->emplace_back(s0, t0, s1, t1, tw, th);
	return tiles->size()-1;

}

void TileSet::set_image(const bitmap &bm) {
	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.width(), bm.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.flipped());
}

int TileSet::add_solid(uint32_t color, uint32_t w, uint32_t h) {

	LOGD("Adding solide %dx%d to pos %d,%d", w, h, xpos, ypos);
	if(ypos + h > texture.height())
		throw tile_exception("Texture full");

	glBindTexture(GL_TEXTURE_2D, texture.id());

	vector<uint32_t> pixels(w*h);
	uint32_t *ptr = &pixels[0];
	for(unsigned int i=0; i<w*h; i++)
		ptr[i] = color;

	double pw = 1.0f/texture.width();
	double ph = 1.0f/texture.height();

	float s0 = xpos * pw;
	float s1 = s0 + pw*tilew;
	float t1 = 1.0 - ypos*ph;
	float t0 = t1 - ph*tileh;

	tiles->emplace_back(s0, t0, s1, t1, tilew, tileh);

	glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
	xpos += w;
	if(xpos+w > texture.width()) {
		xpos = 0;
		ypos += h;
	}

	return tiles->size()-1;
}

void TileSet::render_tile(int tileno, RenderTarget &target, float x, float y, double s) {

	auto &t = (*tiles)[tileno];

	vector<float> uvs = { t.s0, t.t0, t.s1, t.t0, t.s0, t.t1, t.s1, t.t1 };
	target.draw_texture(texture.id(), (int)x, (int)y, t.w, t.h, &uvs[0]);
}

TileLayer::TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts) : 
	scrollx(0), scrolly(0), scale(1.0), tileset(ts), _width(w), _height(h), pixelWidth(pw), pixelHeight(ph), map(w*h), tileSource(nullptr) {}

TileLayer::TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts, function<uint32_t(uint32_t,uint32_t)> source) : 
	scrollx(0), scrolly(0), scale(1.0), tileset(ts), _width(w), _height(h), pixelWidth(pw), pixelHeight(ph), sourceFunction(source), tileSource(nullptr) {
	}

TileLayer::TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts, TileSource &source) : 
	scrollx(0), scrolly(0), scale(1.0), tileset(ts), _width(w), _height(h), pixelWidth(pw), pixelHeight(ph), tileSource(&source) {
	}

void TileLayer::setPixelSize(uint32_t pw, uint32_t ph) {
	pixelWidth = pw;
	pixelHeight = ph;
	if(multiBuf[0] != -1) {
		glDeleteBuffers(2, (GLuint*)multiBuf);
		multiBuf[0] = multiBuf[1] = -1;
	}
}

void TileLayer::render(RenderTarget &target, float x0, float y0) {

	if(tileSource && !tileSource->ready())
		return;

	//pixelWidth = 1600;
	//pixelHeight = 900;

	float s = 1.0;
	int tw = tileset.tilew*s;
	int th = tileset.tileh*s;

	//int pixw = pixelWidth;
	//int pixh = pixelHeight;

	int areaw = (pixelWidth+tw-1) / tw + 1;
	int areah = (pixelHeight+th-1) / th + 1;

	int count = areaw*areah;

	if(multiBuf[0] == -1) {
		glGenBuffers(2, (GLuint*)multiBuf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);
		vector<uint16_t> indexes;//(count*6);
		indexes.reserve(count*6);
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

		int x = x0;
		int y = y0;
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
			x = x0;
		}

		glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);
		glBufferData(GL_ARRAY_BUFFER, coords.size() * 4 * 2, &coords[0], GL_DYNAMIC_DRAW);
	} else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);

	vector<float> uvs(count*8);
	//uvs.resiz(count*8);

	auto scx = scrollx;
	auto scy = scrolly;

	while(scx < 0)
		scx += (tileset.tilew*_width);
	while(scy < 0)
		scy += (tileset.tileh*_height);

	// Start pos in tiles
	int sx = scx / tileset.tilew;
	int sy = scy / tileset.tileh;

	// pixel pos within start tile
	int xx = (int)scx % tileset.tilew;
	int yy = (int)scy % tileset.tileh;

	int i = 0;
	for(int iy=0; iy<areah; iy++) {
		for(int ix=0; ix<areaw; ix++) {
			int xp = (ix+sx) % _width;
			if(xp < 0) xp += _width;
			int yp = (iy+sy) % _height;
			if(yp < 0) yp += _height;
			int tileno;
			if(tileSource) {
				tileno = tileSource->getTile(xp, yp);
			} else if(sourceFunction)
				tileno = sourceFunction(xp, yp);
			else
				tileno = map[xp+yp*_width];

			auto &t = (*tileset.tiles)[tileno];

			uvs[i++] = t.s0;
			uvs[i++] = t.t0;
			uvs[i++] = t.s1;
			uvs[i++] = t.t0;
			uvs[i++] = t.s0;
			uvs[i++] = t.t1;
			uvs[i++] = t.s1;
			uvs[i++] = t.t1;
		}
	}

	//glScissor(0, 100, 1533, 400);
	glEnable(GL_SCISSOR_TEST);

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

	glScissor(x0, target.height()-pixelHeight-y0, pixelWidth, pixelHeight);
	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	//glDisableVertexAttribArray(uvHandle);
	//glDisableVertexAttribArray(posHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_SCISSOR_TEST);
}

void TileLayer::fill(int tileno, int x, int y, int w, int h) {
	if(w == 0) w = _width - x;
	if(h == 0) h = _height -y;
	for(int xx = 0; xx < w; xx++)
		for(int yy = 0; yy < h; yy++)
			map[xx+x+(yy+y)*_width] = tileno;
}

void TileLayer::shift_tiles(int dx, int dy) {
	for(unsigned int i=map.size()-1; i >= _width; i--) {
		map[i] = map[i-_width];
	}
	for(unsigned int i=0; i<_width; i++)
		map[i] = 0;
}

shared_ptr<Sprite> SpriteLayer::addSprite(int tileno, float x, float y, float scale) {
	auto s = make_shared<Sprite>(tileno, x, y, scale);
	sprites.insert(s);
	return s;
}

shared_ptr<Sprite> SpriteLayer::addSprite(vector<int> frames, float x, float y, float scale) {
	auto s = make_shared<Sprite>(frames, x, y, scale);
	sprites.insert(s);
	return s;
}

void SpriteLayer::render(RenderTarget &target, int x, int y) {

	glBindTexture(GL_TEXTURE_2D, tileSet.texture.id());

	//glScissor(x, target.height()-pixelHeight-y, pixelWidth, pixelHeight);
	//glEnable(GL_SCISSOR_TEST);

	//glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);

	auto i = sprites.begin();
	while(i != sprites.end()) {
		auto s = i->lock();
		if(s) {
			auto &t = s->tileno < 0 ? (*tileSet.tiles)[s->frames[s->frame]] : (*tileSet.tiles)[s->tileno];
			vector<float> uvs = { t.s0, t.t0, t.s1, t.t0, t.s0, t.t1, t.s1, t.t1 };
			target.draw_texture(-1, (int)s->x + x - scrollx, (int)s->y + y - scrolly, t.w * s->scale, t.h * s->scale, &uvs[0]);
			//tileSet.render_tile(s->tileno, target, s->x + x - scrollx, s->y + y - scrolly, s->scale);
			++i;
		} else 
			i = sprites.erase(i);
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_DEPTH_TEST);
}

void SpriteLayer::foreach(function<void(Sprite&)> f) {
	for(auto &ws : sprites) {
		auto s = ws.lock();
		if(s)
			f(*s);
	}
}

void SpriteLayer::setPixelSize(uint32_t pw, uint32_t ph) {
	pixelWidth = pw;
	pixelHeight = ph;
}


}