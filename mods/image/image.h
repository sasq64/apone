#ifndef IMAGE_IMAGE_H
#define IMAGE_IMAGE_H

#include "bitmap.h"
#include <stdexcept>
#include <vector>

namespace image {

class image_exception : public std::exception {
public:
	image_exception(const std::string &msg) : msg(msg) {}
	virtual const char *what() const throw() { return msg.c_str(); }
private:
	std::string msg;
};

std::vector<bitmap> load_gifs(const std::string &filename);
bitmap load_image(const std::string &file_name);
//bitmap load_png(const std::string &file_name);
void save_png(bitmap bitmap, const std::string &path);

}

#endif // IMAGE_IMAGE_H
