#include "tiles.h"
#include "shader.h"
#include "transform.h"
#include "window.h"
#include <coreutils/log.h>
#include <image/image.h>
#include <image/packer.h>
#include <cmath>
#include <vector>

using namespace std;
using namespace image;
using namespace utils;

namespace grappix {

//TileSet::TileSet() {}

TileSet::TileSet(shared_ptr<ImagePacker> packer) : packer(packer), texture(packer->size().w, packer->size().h) {

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}

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
	add(xpos, ypos, tilew, tileh);

	glBindTexture(GL_TEXTURE_2D, texture.id());
	glTexSubImage2D(GL_TEXTURE_2D, 0, xpos, texture.height()-tileh-ypos, tilew, tileh, GL_RGBA, GL_UNSIGNED_BYTE, bm.flipped());

	return tiles.size()-1;
}

int TileSet::add(int tx, int ty, int tw, int th) {

	double pw = 1.0f/texture.width();
	double ph = 1.0f/texture.height();

	float s0 = tx * pw;
	float s1 = s0 + pw*tw;
	float t1 = 1.0 - ty*ph;
	float t0 = t1 - ph*th;

	tiles.emplace_back(s0, t0, s1, t1, tw, th);
	return tiles.size()-1;

}

void TileSet::renderTile(int tileno, RenderTarget &target, float x, float y, double s) {

	auto &t = tiles[tileno];
	vector<float> uvs = { t.s0, t.t0, t.s1, t.t0, t.s0, t.t1, t.s1, t.t1 };
	target.draw_texture(texture.id(), (int)x, (int)y, t.w, t.h, &uvs[0]);
}

void save_data(File &f, const TileSet &data) {

	const auto &bm = data.texture.get_pixels();
	image::save_png(bm, f.getName());
	File mf { f.getName() + ".tiles" };
	for(const auto &t : data.tiles) {
		mf.write(format("%f %f %f %f %d %d\n", t.s0, t.t0, t.s1, t.t1, t.w, t.h));
	}
	mf.close();
}


#ifdef ANDROID
#define stof(x) strtod(x.c_str(), nullptr)
#endif

template <> shared_ptr<TileSet> load_data(File &f) {
	auto ts = make_shared<TileSet>();
	auto bm = image::load_png(f.getName());
	ts->texture = Texture(bm);
	File mf { f.getName() + ".tiles" };
	for(const auto &t : mf.getLines()) {
		auto p = split(t, " ");
		ts->tiles.emplace_back(stof(p[0]), stof(p[1]), stof(p[2]), stof(p[3]), stol(p[4]), stol(p[5]));
	}
	mf.close();
	return ts;
}















//TileLayer::TileLayer(uint32_t pw, uint32_t ph, uint32_t tw, uint32_t th, const TileSet &ts) :
//	scrollx(0), scrolly(0), scale(1.0), tileset(ts), pixel_width(pw), pixel_height(ph), tile_width(tw), tile_height(th), tileSource(nullptr) {}

TileLayer::TileLayer(shared_ptr<TileSet> ts, shared_ptr<TileSource> source, uint32_t tw, uint32_t th) :
	scrollx(0), scrolly(0), scale(1.0), tileset(ts), tile_width(tw), tile_height(th), tileSource(source) {
		if(ts) {
			if(tile_width == 0)
				tile_width = ts->tiles[0].w;
			if(tile_height == 0)
				tile_height = ts->tiles[0].h;
		}
		frame = Frame(screen.rec());
	}


void TileLayer::render(RenderTarget &target) {

	if(!tileSource || !tileset) // && !tileSource->ready())
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
			if(tileno >= tileset->size()) tileno = 0;

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
	glBindTexture(GL_TEXTURE_2D, tileset->get_texture().id());

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

	if(frame)
		frame.set(target);

	//glScissor(x0, target.height()-pixel_height-y0, pixel_width, pixel_height);
	glDrawElements(GL_TRIANGLES, 6*count, GL_UNSIGNED_SHORT, 0);

	if(frame)
		frame.unset();

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

	if(!tileset)
		return;

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
			target.draw(tileset->get_texture(), (int)s->x + x - scrollx, (int)s->y + y - scrolly, t.w * s->scale, t.h * s->scale, &uvs[0]);
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