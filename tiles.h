#ifndef TILES_H
#define TILES_H

#include "texture.h"
#include "bitmap.h"

#include <vector>

struct TileSet {

	TileSet();
	TileSet(int tilew, int tileh);
	int add_tiles(const bitmap &bm);

	void render_tile(int tileno, basic_buffer &target, float x, float y, double s);

	int tilew;
	int tileh;
	int widthInTiles;
	int heightInTiles;
	texture texture;
};


class TileLayer {
public:
	TileLayer(int w, int h, int pw, int ph, TileSet &ts);
	void render(basic_buffer &target, int x = 0, int y = 0);
	void render2(basic_buffer &target, float x = 0, float y = 0);

	//float scale() { return s; }
	//float scale(float s) { this->s = s; return s; }
	int width() const { return _width; }
	int height() const { return _height; }

	uint32_t &operator[](uint32_t i) {
		return map[i];
	}

	// Pixel offset into tiles
	int scrollx;
	int scrolly;
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

	void render(basic_buffer &target) {
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
