#include "tiles.h"
#include "shader.h"
#include "transform.h"

#include <coreutils/log.h>
#include <image/packer.h>
#include <cmath>
#include <vector>

using namespace std;
using namespace image;
using namespace utils;

namespace grappix {

//TileSet::TileSet() {}

TileSet::TileSet(shared_ptr<ImagePacker> packer) : packer(packer) {}

TileSet::TileSet(uint32_t texw, uint32_t texh) : texture(texw, texh)  {

	packer = make_shared<SequentialPacker>(texw, texh);

	//tiles = make_shared<vector<tile>>();

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

int TileSet::add(const bitmap &bm) {

	int tilew = bm.width();
	int tileh = bm.height();

	ImagePacker::Rect r(0, 0, tilew, tileh);
	if(!packer->add(r))
		throw tile_exception("Texture full");

	auto xpos = r.x;
	auto ypos = r.y;

	//LOGD("Adding %dx%d to pos %d,%d", tilew, tileh, xpos, ypos);

	double pw = 1.0/texture.width();
	double ph = 1.0/texture.height();

	float s0 = xpos * pw;
	float s1 = s0 + pw*tilew;
	float t1 = 1.0 - ypos*ph;
	float t0 = t1 - ph*tileh;

	tiles.emplace_back(s0, t0, s1, t1, tilew, tileh);

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, texture.height()-tileh-ypos, tilew, tileh, GL_RGBA, GL_UNSIGNED_BYTE, bm.flipped());

	return tiles.size()-1;
}

int TileSet::add_tile(int tx, int ty, int tw, int th) {

	//auto p = 1.0 / utils::make_vec(texture.width(), texture.height());

	double pw = 1.0f/texture.width();
	double ph = 1.0f/texture.height();

	float s0 = tx * pw;
	float s1 = s0 + pw*tw;
	float t1 = 1.0 - ty*ph;
	float t0 = t1 - ph*th;

	tiles.emplace_back(s0, t0, s1, t1, tw, th);
	return tiles.size()-1;

}

void TileSet::set_image(const bitmap &bm) {
	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.width(), bm.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.flipped());
}

int TileSet::add_solid(uint32_t color, uint32_t w, uint32_t h) {

	ImagePacker::Rect r(0, 0, w, h);
	if(!packer->add(r))
		throw tile_exception("Texture full");
	auto xpos = r.x;
	auto ypos = r.y;

	LOGD("Adding solid %dx%d to pos %d,%d", w, h, xpos, ypos);

	glBindTexture(GL_TEXTURE_2D, texture.id());

	vector<uint32_t> pixels(w*h);
	uint32_t *ptr = &pixels[0];
	for(unsigned int i=0; i<w*h; i++)
		ptr[i] = color;

	double pw = 1.0f/texture.width();
	double ph = 1.0f/texture.height();

	float s0 = xpos * pw;
	float s1 = s0 + pw*w;
	float t1 = 1.0 - ypos*ph;
	float t0 = t1 - ph*h;

	tiles.emplace_back(s0, t0, s1, t1, w, h);

	glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, ypos, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

	return tiles.size()-1;
}

void TileSet::render_tile(int tileno, RenderTarget &target, float x, float y, double s) {

	auto &t = tiles[tileno];

	vector<float> uvs = { t.s0, t.t0, t.s1, t.t0, t.s0, t.t1, t.s1, t.t1 };
	target.draw_texture(texture.id(), (int)x, (int)y, t.w, t.h, &uvs[0]);
}

//TileLayer::TileLayer(uint32_t pw, uint32_t ph, uint32_t tw, uint32_t th, const TileSet &ts) : 
//	scrollx(0), scrolly(0), scale(1.0), tileset(ts), pixel_width(pw), pixel_height(ph), tile_width(tw), tile_height(th), tileSource(nullptr) {}

TileLayer::TileLayer(shared_ptr<TileSet> ts, TileSource &source, uint32_t tw, uint32_t th) :
	scrollx(0), scrolly(0), scale(1.0), tileset(ts), tile_width(tw), tile_height(th), tileSource(&source) {
	}

/*
void TileLayer::setPixelSize(uint32_t pw, uint32_t ph) {
	pixel_width = pw;
	pixel_height = ph;
	if(multiBuf[0] != -1) {
		glDeleteBuffers(2, (GLuint*)multiBuf);
		multiBuf[0] = multiBuf[1] = -1;
	}
}
*/
void TileLayer::render(RenderTarget &target) {

	if(tileSource && !tileSource->ready())
		return;

	auto rec = frame.rec();
	float x0 = rec.x;
	float y0 = rec.y;
	uint32_t pixel_width = rec.w;
	uint32_t pixel_height = rec.h;

	int areaw = (pixel_width+tile_width-1) / tile_width + 1;
	int areah = (pixel_height+tile_height-1) / tile_height + 1;

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
				coords.push_back(y+tile_height);
				coords.push_back(x+tile_width);
				coords.push_back(y+tile_height);
				coords.push_back(x);
				coords.push_back(y);
				coords.push_back(x+tile_width);
				coords.push_back(y);
				x += tile_width;
			}
			y += tile_height;
			x = x0;
		}

		glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);
		glBufferData(GL_ARRAY_BUFFER, coords.size() * 4 * 2, &coords[0], GL_DYNAMIC_DRAW);
	} else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, multiBuf[1]);

	vector<float> uvs(count*8);

	// Start pos in tiles
	int sx = scrollx / tile_width;
	int sy = scrolly / tile_height;

	// pixel pos within start tile
	int xx = (int)scrollx % tile_width;
	int yy = (int)scrolly % tile_height;

	int i = 0;
	for(int iy=0; iy<areah; iy++) {
		for(int ix=0; ix<areaw; ix++) {
			auto tileno = tileSource->getTile(ix+sx, iy+sy);
			//LOGD("TILE %d", tileno);
			if(tileno >= tileset->tiles.size()) tileno = 0;

			const auto &t = tileset->tiles[tileno];

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
	//glEnable(GL_SCISSOR_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, multiBuf[0]);
	glBufferSubData(GL_ARRAY_BUFFER, count*8*4, uvs.size() * 4, &uvs[0]);

	auto &program = get_program(TEXTURED_PROGRAM);
	program.use();

	glBindFramebuffer(GL_FRAMEBUFFER, target.buffer());
	glViewport(0,0,target.width(), target.height());
	glBindTexture(GL_TEXTURE_2D, tileset->texture.id());

	//auto sx = globalScale * w/2;
	//auto sy = globalScale * h/2;
	mat4f matrix(1.0);

	
	//matrix = make_rotate_z(50.0) * matrix;
	//matrix = make_scale(sx, sy) * matrix;
	matrix = make_translate(-xx, -yy) * matrix;
	matrix = make_scale(2.0 / target.width(), 2.0 / target.height()) * matrix;
	matrix = make_scale(1.0, -1.0) * matrix;
	matrix = make_translate(-1.0, 1.0, 0.5) * matrix;

	program.setUniform("matrix", matrix.transpose());

	// program.setUniform("vScreenScale", 2.0 / target.width(), 2.0 / target.height(), 0, 1);
	// program.setUniform("vScale", 1.0, 1.0, 0, 1);
	// program.setUniform("vPosition", -xx, -yy, 0, 1);

	program.vertexAttribPointer("vertex", 2, GL_FLOAT, GL_FALSE, 0, 0);
	program.vertexAttribPointer("uv", 2, GL_FLOAT, GL_FALSE, 0, count*8*4);

	//glScissor(x0, target.height()-pixel_height-y0, pixel_width, pixel_height);
	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	//glDisableVertexAttribArray(uvHandle);
	//glDisableVertexAttribArray(posHandle);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisable(GL_SCISSOR_TEST);
}

void SpriteLayer::purgeSprites() {
	auto i = sprites.begin();
	while(i != sprites.end()) {
		auto s = i->lock();
		if(i->lock())
			++i;
		else 
			i = sprites.erase(i);
	}
}

shared_ptr<Sprite> SpriteLayer::addSprite(int tileno, float x, float y, float scale) {
	purgeSprites();
	auto s = make_shared<Sprite>(tileno, x, y, scale);
	sprites.insert(s);
	return s;
}

shared_ptr<Sprite> SpriteLayer::addSprite(vector<int> frames, float x, float y, float scale) {
	purgeSprites();
	auto s = make_shared<Sprite>(frames, x, y, scale);
	sprites.insert(s);
	return s;
}

void SpriteLayer::render(RenderTarget &target, int x, int y) {

	//glBindTexture(GL_TEXTURE_2D, tileset->texture.id());

	//if(pixel_width >= 0) {
	//	glScissor(x, target.height()-pixel_height-y, pixel_width, pixel_height);
	//	glEnable(GL_SCISSOR_TEST);
	//}

	//glClear(GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_DEPTH_TEST);
	if(frame)
		frame.set(target);
	auto i = sprites.begin();
	while(i != sprites.end()) {
		//LOGD("x");
		auto s = i->lock();
		if(s) {
			auto &t = s->tileno < 0 ? tileset->tiles[s->frames[s->frame]] : tileset->tiles[s->tileno];
			vector<float> uvs = { t.s0, t.t0, t.s1, t.t0, t.s0, t.t1, t.s1, t.t1 };
			target.draw(tileset->texture, (int)s->x + x - scrollx, (int)s->y + y - scrolly, t.w * s->scale, t.h * s->scale, &uvs[0]);
			++i;
		} else 
			i = sprites.erase(i);
	}
	if(frame)
		frame.unset();
	//if(pixel_width >= 0)
	//	glDisable(GL_SCISSOR_TEST);

	//glDisable(GL_DEPTH_TEST);
}

// void SpriteLayer::foreach(function<void(Sprite&)> f) {
// 	for(auto &ws : sprites) {
// 		auto s = ws.lock();
// 		if(s)
// 			f(*s);
// 	}
// }

//void SpriteLayer::setPixelSize(uint32_t pw, uint32_t ph) {
//	pixel_width = pw;
//	pixel_height = ph;
//}


}