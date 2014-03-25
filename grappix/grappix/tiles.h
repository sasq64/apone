#ifndef TILES_H
#define TILES_H

#include "texture.h"
#include "bitmap.h"

#include <vector>

namespace grappix {

class tile_exception : public std::exception {
public:
	tile_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

struct TileSet {

	TileSet();
	TileSet(unsigned int tilew, unsigned int tileh, uint32_t texh = 256, uint32_t texw = 256);
	int add_tiles(const bitmap &bm);
	void set_image(const bitmap &bm);

	int add_solid(uint32_t color);
	void set_tile(int no) {
		xpos = (no % widthInTiles) * tilew;
		ypos = (no / widthInTiles) * tileh;
	}

	void render_tile(int tileno, RenderTarget &target, float x, float y, double s);

	unsigned int tilew;
	unsigned int tileh;
	unsigned int widthInTiles;
	unsigned int heightInTiles;
	Texture texture;

	// Current add position
	unsigned int xpos;
	unsigned int ypos;
};


class TileLayer {
public:
	TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts);
	TileLayer(uint32_t w, uint32_t h, uint32_t pw, uint32_t ph, const TileSet &ts, std::function<uint32_t(uint32_t x, uint32_t y)> source);
	void render(RenderTarget &target, float x = 0, float y = 0);

	//float scale() { return s; }
	//float scale(float s) { this->s = s; return s; }
	uint32_t width() const { return _width; }
	uint32_t height() const { return _height; }
	uint32_t size() const { return _width * _height;  }

	void fill(int tileno = 0, int x = 0, int y = 0, int w = 0, int h = 0) {
		if(w == 0) w = _width - x;
		if(h == 0) h = _height -y;
		for(int xx = 0; xx < w; xx++)
			for(int yy = 0; yy < h; yy++)
				map[xx+x+(yy+y)*_width] = tileno;
	}

	void shift_tiles(int dx, int dy) {
		for(unsigned int i=map.size()-1; i >= _width; i--) {
			map[i] = map[i-_width];
		}
		for(unsigned int i=0; i<_width; i++)
			map[i] = 0;
	}

	uint32_t &operator[](uint32_t i) {
		return map[i];
	}

	// Pixel offset into tiles
	float scrollx;
	float scrolly;
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

	int multiBuf[2] = {-1, -1};
};

struct Sprite {
	Sprite(int tileno = 0, float x = 0.0f, float y = 0.0f, float scale = 1.0f) : tileno(tileno), x(x), y(y), scale(scale) {}
	int tileno;
	float x;
	float y;
	float scale;
	int frame;
};

class SpriteLayer {
public:
	SpriteLayer(const TileSet &ts) : tileSet(ts) {}
	std::shared_ptr<Sprite> addSprite(int tileno, float x = 0.0, float y = 0.0, float scale = 1.0) {
		auto s = std::make_shared<Sprite>(tileno, x, y, scale);
		sprites.push_back(s);
		return s;
	}

	void render(RenderTarget &target, int x = 0, int y = 0) {

		auto i = sprites.begin();
		while(i != sprites.end()) {
			auto s = i->lock();
			if(s) {
				tileSet.render_tile(s->tileno, target, s->x + x, s->y + y, s->scale);
				++i;
			} else 
				i = sprites.erase(i);
		}
	}

	std::shared_ptr<Sprite> operator[](const int &i) { 
		return sprites[i].lock();
	}
	
	const std::shared_ptr<Sprite> operator[](const int &i) const { return sprites[i].lock(); }

	void foreach(std::function<void(Sprite&)> f) {
		for(auto &ws : sprites) {
			auto s = ws.lock();
			if(s)
				f(*s);
		}
	}

private:
	std::vector<std::weak_ptr<Sprite>> sprites;
	TileSet tileSet;
};

}
#endif // TILES_H
