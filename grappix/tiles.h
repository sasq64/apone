#ifndef TILES_H
#define TILES_H

#include "texture.h"
#include "bitmap.h"

#include <vector>

class tile_exception : public std::exception {
public:
	tile_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

struct TileSet {

	TileSet();
	TileSet(int tilew, int tileh);
	int add_tiles(const bitmap &bm);

	void render_tile(int tileno, RenderTarget &target, float x, float y, double s);

	int tilew;
	int tileh;
	int widthInTiles;
	int heightInTiles;
	Texture texture;
};


class TileLayer {
public:
	TileLayer(int w, int h, int pw, int ph, TileSet &ts);
	void render(RenderTarget &target, float x = 0, float y = 0);

	//float scale() { return s; }
	//float scale(float s) { this->s = s; return s; }
	int width() const { return _width; }
	int height() const { return _height; }

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
	int _width;
	int _height;
	// Size of visible area
	int pixelWidth;
	int pixelHeight;

	std::vector<uint32_t> map;

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
	SpriteLayer(TileSet &ts) : tileSet(ts) {}
	int addSprite(int tileno, float x = 0.0, float y = 0.0, float scale = 1.0) {
		sprites.emplace_back(tileno, x, y, scale);
		return sprites.size() -1;
	}

	Sprite &sprite(int i) { return sprites[i]; }

	void render(RenderTarget &target) {
		for(auto &s : sprites) {
			tileSet.render_tile(s.tileno, target, s.x, s.y, s.scale);
		}
	}

	Sprite& operator[](const int &i) { 
		return sprites[i];
	}
	
	const Sprite& operator[](const int &i) const { return sprites[i]; }

	void foreach(std::function<void(Sprite&)> f) {
		for(auto &s : sprites) {
			f(s);
		}
	}

private:
	std::vector<Sprite> sprites;
	TileSet tileSet;
};


#endif // TILES_H
