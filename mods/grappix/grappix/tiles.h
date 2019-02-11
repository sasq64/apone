#ifndef TILES_H
#define TILES_H

#include "frame.h"
#include "texture.h"
#include "resources.h"
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

struct TileSet {

	TileSet(uint32_t texw = 256, uint32_t texh = 256);
	TileSet(std::shared_ptr<image::ImagePacker> packer);

	int add(const image::bitmap &bm);
	int add(int x, int y, int w, int h);

	int addSolid(uint32_t color, uint32_t w, uint32_t h) {
		return add(image::bitmap(w, h, color));
	}

	void renderTile(int tileno, RenderTarget &target, float x, float y, double s = 1.0);

	Texture get_texture() const {
		return texture;
	}

	size_t size() const { return tiles.size(); }

	struct tile {
		tile(float s0, float t0, float s1, float t1, int w, int h) : s0(s0), t0(t0), s1(s1), t1(t1), w(w), h(h) {}
		float s0, t0, s1, t1;
		int w, h;
	};

	std::vector<tile> tiles;

private:
	std::shared_ptr<image::ImagePacker> packer;
	Texture texture;

	friend void save_data(utils::File &f, const TileSet &data);
	template <typename T> friend std::shared_ptr<T> load_data(utils::File &f);

};

void save_data(utils::File &f, const TileSet &data);
TileSet load_data(utils::File &f);


class TileSource : public std::enable_shared_from_this<TileSource> {
public:
    virtual ~TileSource() {}
	virtual uint32_t getTile(uint32_t, uint32_t) = 0;
};


class TileArray : public TileSource {
public:
    virtual ~TileArray() {}
	TileArray(uint32_t w, uint32_t h) : _width(w), _height(h), _size(w*h), tiles(_size) {}
	virtual uint32_t getTile(uint32_t x , uint32_t y) { return tiles[(x%_width) + (y%_height) * _width]; }
	//bool ready() { return true; }

	uint32_t width() const { return _width; }
	uint32_t height() const { return _height; }
	size_t size() const { return _size; }

	void fill(int tileno, int x = 0, int y = 0, int w = 0, int h = 0) {
		if(w == 0) w = _width - x;
		if(h == 0) h = _height -y;
		for(int xx = 0; xx < w; xx++)
			for(int yy = 0; yy < h; yy++)
				tiles[xx+x+(yy+y)*_width] = tileno;
	}

	void shift_tiles() {
		for(unsigned int i=tiles.size()-1; i >= _width; i--) {
			tiles[i] = tiles[i-_width];
		}
		for(unsigned int i=0; i<_width; i++)
			tiles[i] = 0;
	}

	std::vector<uint32_t>::iterator begin() {
		return tiles.begin();
	}

	std::vector<uint32_t>::iterator end() {
		return tiles.end();
	}

	uint32_t& at(int x, int y = 0) {
		return tiles[(x%_width)+ (y%_height) * _width];
	}

	uint32_t operator[](uint32_t o) const {
		return tiles[o % _size];
	}

	uint32_t& operator[](uint32_t o) {
		return tiles[o % _size];
	}

	void transform(std::function<void(int &x, int &y, int t)> func) {
		decltype(tiles) newtiles;
		newtiles.resize(tiles.size());
		delta.resize(tiles.size());
		for(int y=0; y<8; y++)
			for(int x=0; x<8; x++) {
				int xx = x;
				int yy = y;
				auto o = x+y*_width;
				func(xx, yy, tiles[o]);
				xx = (xx % _width);
				yy = (yy % _height);
				delta[o] = std::make_pair(xx - x, yy - y);
				newtiles[o] = tiles[xx + yy * _width];
			}

		tiles = newtiles;
	}

	std::pair<short, short> get_delta(int i) { return delta[i]; }

private:
	uint32_t _width;
	uint32_t _height;
	size_t _size;
	std::vector<uint32_t> tiles;
	std::vector<std::pair<short, short>> delta;

};

class TileLayer {
public:
	TileLayer(std::shared_ptr<TileSet> ts, std::shared_ptr<TileSource> source, uint32_t tw = 0, uint32_t th = 0);
	void render(RenderTarget &target);

	void setFrame(const Frame &f) { frame = f; }

	void setTiles(std::shared_ptr<TileSet> ts) {
		tileset = ts;
		if(tile_width == 0)
			tile_width = ts->tiles[0].w;
		if(tile_height == 0)
			tile_height = ts->tiles[0].h;
	}

	uint32_t tileWidth() { return tile_width; }
	uint32_t tileHeight() { return tile_height; }

	// Pixel offset into tiles
	double scrollx;
	double scrolly;
	float scale;

private:

	std::shared_ptr<TileSet> tileset;
	Frame frame;

	uint32_t tile_width;
	uint32_t tile_height;

	std::shared_ptr<TileSource> tileSource;

	int multiBuf[2] = {-1, -1};
};

struct Sprite {
	Sprite(int tileno = 0, float x = 0.0f, float y = 0.0f, float scale = 1.0f) : tileno(tileno), x(x), y(y), scale(scale), frame(0) {
		//LOGD("Sprite created");
	}
	Sprite(std::vector<int> frames, float x = 0.0f, float y = 0.0f, float scale = 1.0f) : frames(frames), tileno(-1), x(x), y(y), scale(scale), frame(0) {
		//LOGD("Sprite created");
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
	SpriteLayer(std::shared_ptr<TileSet> ts) : scrollx(0), scrolly(0), tileset(ts) {}
	std::shared_ptr<Sprite> addSprite(std::vector<int> frames, float x = 0.0, float y = 0.0, float scale = 1.0);
	std::shared_ptr<Sprite> addSprite(int tileno, float x = 0.0, float y = 0.0, float scale = 1.0);
	void render(RenderTarget &target, int x = 0, int y = 0);
	//void foreach(std::function<void(Sprite&)> f);
	//void setPixelSize(uint32_t px, uint32_t ph);
	void setTiles(std::shared_ptr<TileSet> ts) {
		tileset = ts;
	}

	void setFrame(const Rectangle &rec) { frame = Frame(rec); }
	void setFrame(const Frame &f) { frame = f; }

	double scrollx;
	double scrolly;
private:
	void purgeSprites();
	std::multiset<std::weak_ptr<Sprite>, SpriteCompare> sprites;
	std::shared_ptr<TileSet> tileset;

	Frame frame;

};

}
#endif // TILES_H
