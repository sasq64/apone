#ifndef TILES_H
#define TILES_H

#include "texture.h"
#include <image/bitmap.h>
#include <image/packer.h>

#include <vector>
#include <set>
namespace grappix {

class tile_exception : public std::exception {
public:
	tile_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};
/*
struct SimplePacker {
	SimplePacker(int w, int h) : xpos(0), ypos(0), width(w), height(h) {}
	std::pair<int, int> add(int w, int h) {

		if(ypos + w > height)
			throw tile_exception("Texture full");

		auto rc = make_pair(xpos, ypos);
		xpos += w;
		if(xpos > width) {
			xpos = 0;
			ypos += h;
		}
		return rc;
	}
	void free(int x, int y) {
		throw tile_exception("Free not supported");
	}
	int xpos;
	int ypos;
	int width;
	int height;
};
*/

struct TileSet {

	TileSet();
	TileSet(unsigned int tilew, unsigned int tileh, uint32_t texw = 256, uint32_t texh = 256);
	int add_tiles(const image::bitmap &bm) {
		for(const auto &b : bm.split(tilew, tileh)) {
			add(b);
		}
		return tiles->size()-1;
	} 

	int add(const image::bitmap &bm);
	//int add(const bitmap::splitter &
	void set_image(const image::bitmap &bm);

	int add_solid(uint32_t color, uint32_t w, uint32_t h);
	void set_tile(unsigned int no) {
		while(tiles->size() < no)
			tiles->emplace_back(0,0,0,0,0,0);
	}

	int add_tile(int x, int y, int w, int h);

	void render_tile(int tileno, RenderTarget &target, float x, float y, double s);

	unsigned int tilew;
	unsigned int tileh;
	//unsigned int widthInTiles;
	//unsigned int heightInTiles;
	Texture texture;

	// Current add position
	unsigned int xpos;
	unsigned int ypos;

	struct tile {
		tile(float s0, float t0, float s1, float t1, int w, int h) : s0(s0), t0(t0), s1(s1), t1(t1), w(w), h(h) {}
		float s0, t0, s1, t1;
		int w, h;
	};

	std::shared_ptr<std::vector<tile>> tiles;
	std::shared_ptr<image::Packer> packer;
};


class TileLayer {
public:

	class TileSource {
	public:
		virtual uint32_t getTile(uint32_t, uint32_t) = 0;
		virtual bool ready() = 0;
	};

	TileLayer() {}

	TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts);
	TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts, std::function<uint32_t(uint32_t x, uint32_t y)> source);
	TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts, TileSource &source);
	void render(RenderTarget &target, float x = 0, float y = 0);

	void setPixelSize(uint32_t px, uint32_t ph);

	uint32_t pixel_width() { return pixelWidth; }
	uint32_t pixel_height() { return pixelHeight; }

	//float scale() { return s; }
	//float scale(float s) { this->s = s; return s; }
	uint32_t width() const { return _width; }
	uint32_t height() const { return _height; }
	uint32_t size() const { return _width * _height;  }

	void fill(int tileno = 0, int x = 0, int y = 0, int w = 0, int h = 0);
	void shift_tiles(int dx, int dy);

	uint32_t &operator[](uint32_t i) {
		return map[i];
	}

	// Pixel offset into tiles
	double scrollx;
	double scrolly;
	float scale;

private:

	TileSet tileset;

	// Total size in tiles
	uint32_t _width;
	uint32_t _height;
	// Size of visible area
	uint32_t pixelWidth;
	uint32_t pixelHeight;

	std::vector<uint32_t> map;
	std::function<int(int,int)> sourceFunction;
	TileSource *tileSource;

	int multiBuf[2] = {-1, -1};
};

struct Sprite {
	Sprite(int tileno = 0, float x = 0.0f, float y = 0.0f, float scale = 1.0f) : tileno(tileno), x(x), y(y), scale(scale), frame(0) {
		LOGD("Sprite created");
	}
	Sprite(std::vector<int> frames, float x = 0.0f, float y = 0.0f, float scale = 1.0f) : frames(frames), tileno(-1), x(x), y(y), scale(scale), frame(0) {
		LOGD("Sprite created");
	}
	//~Sprite() { LOGD("Sprite destroyed"); }
	std::vector<int> frames;

	int tileno;

	float x;
	float y;
	float scale;
	int frame;
};


class SpriteLayer {
	struct SpriteCompare {
		bool operator()(const std::weak_ptr<Sprite> &lhs, const std::weak_ptr<Sprite> &rhs) const {
			return lhs.lock()->y < rhs.lock()->y;
		}
	};
public:
	SpriteLayer() {}
	SpriteLayer(const TileSet &ts, int pw, int ph) : scrollx(0), scrolly(0), tileSet(ts), pixelWidth(pw), pixelHeight(ph) {}
	std::shared_ptr<Sprite> addSprite(std::vector<int> frames, float x = 0.0, float y = 0.0, float scale = 1.0);
	std::shared_ptr<Sprite> addSprite(int tileno, float x = 0.0, float y = 0.0, float scale = 1.0);
	void render(RenderTarget &target, int x = 0, int y = 0);
	void foreach(std::function<void(Sprite&)> f);
	void setPixelSize(uint32_t px, uint32_t ph);

	double scrollx;
	double scrolly;
private:
	std::set<std::weak_ptr<Sprite>, SpriteCompare> sprites;
	TileSet tileSet;
	int pixelWidth;
	int pixelHeight;

};

}
#endif // TILES_H
