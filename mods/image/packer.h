#ifndef IMAGE_PACKER_H
#define IMAGE_PACKER_H

#include <cstdint>
#include <memory>

#include <coreutils/utils.h>
#include <coreutils/log.h>

namespace image {

class packer_exception : public std::exception {
public:
	packer_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

class ImagePacker {
public:
	struct Rect {
		Rect(int x = 0, int y = 0, int w = 0, int h = 0) : x(x), y(y), w(w), h(h) {}
		uint16_t x;
		uint16_t y;
		uint16_t w;
		uint16_t h;
	};

	virtual ~ImagePacker() = default;

	virtual bool add(Rect &r) = 0;
	virtual void remove(const Rect &r) = 0;

	virtual Rect size() = 0;
};

class SequentialPacker : public ImagePacker {
public:
	SequentialPacker(int w, int h) : xpos(0), ypos(0), width(w), height(h) {}
	bool add(Rect &r) override {

		if(ypos + r.w > height)
			return false;

		r.x = xpos;
		r.y = ypos;

		xpos += r.w;
		if(xpos >= width) {
			xpos = 0;
			ypos += r.h;
		}
		return true;
	}
	void remove(const Rect &/*r*/) override {
		throw packer_exception("remove() not supported in SequentialPacker");
	}

	Rect size() override { return Rect(0,0,width,height); }

private:
	int xpos;
	int ypos;
	int width;
	int height;
};

class KDPacker : public ImagePacker {
public:

	struct Node {
		Node(const Rect &r, bool used) : r(r), used(used) {}

		bool remove(const Rect &box) {

			if(used && box.x == r.x && box.y == r.y && box.w == r.w && box.h == r.h) {
				used = false;
				//LOGD("Removing box");
				return true;
			}

			if(!kids[0])
				return true;

			bool r0 = kids[0]->remove(box);
			bool r1 = kids[1]->remove(box);

			// If both kids are leaves
			if(r0 && r1) {
				if(kids[0]->used || kids[1]->used)
					return false;
				//LOGD("Joining %d,%d %dx%d and %d,%d %dx%d", kids[0]->r.x, kids[0]->r.y, kids[0]->r.w, kids[0]->r.h,
				//kids[1]->r.x, kids[1]->r.y, kids[1]->r.w, kids[1]->r.h);
				kids[0] = nullptr;
				kids[1] = nullptr;
				used = false;
				return true;
			}
			return false;
		}

		bool add(Rect &box) {
			if(used) {
				//LOGD("Rect (%d,%d,%d,%d) used", r.x, r.y, r.w, r.h);
				return false;
			}

			// If we have kids, visit them
			if(kids[0]) {
				//LOGD("Checking kids");
				bool rc = kids[0]->add(box);
				if(rc) return true;
				return kids[1]->add(box);
			}

			//LOGD("Trying (%d,%d) against this rect (%d,%d,%d,%d)", box.w, box.h, r.x, r.y, r.w, r.h);

			// If we cant fit our box inside this rect, return
			if(box.w > r.w || box.h > r.h) {
				//LOGD("Wont fit");
				return false;
			}

			// Use this
			Rect r0 = r;
			Rect r1 = r;
			if(r.w == box.w && r.h == box.h) {
				//LOGD("Exact fit");
				used = true;
			} else if(r.w == box.w) {
				// Split horizontally
				r0.h = box.h;
				r1.y += box.h;
				r1.h -= box.h;
				kids[0] = std::make_unique<Node>(r0, true);
				kids[1] = std::make_unique<Node>(r1, false);
			} else if(r.h == box.h) {
				// Split vertically
				r0.w = box.w;
				r1.x += box.w;
				r1.w -= box.w;
				kids[0] = std::make_unique<Node>(r0, true);
				kids[1] = std::make_unique<Node>(r1, false);
			} else {
				// Split horizontally
				//LOGD("Splitting box");
				r0.h = box.h;
				r1.y += box.h;
				r1.h -= box.h;
				kids[0] = std::make_unique<Node>(r0, false);
				kids[1] = std::make_unique<Node>(r1, false);
				return kids[0]->add(box);
			}
			box = r0;
			return true;
		}

		Rect r;
		std::unique_ptr<Node> kids[2];
		bool used; // Should always false if node have kids

	};

	KDPacker(const Rect &r) : root(r, false) {}

	KDPacker(int w, int h) : root(Rect(0,0,w,h), false) {}

	bool add(Rect &r) override {
		return root.add(r);
	}

	void remove(const Rect &r) override{
		root.remove(r);
	}

	virtual Rect size() override { return root.r; }

private:


	Node root;
};

}

#endif // IMAGE_PACKER_H
