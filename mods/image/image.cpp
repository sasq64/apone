#include "image.h"

#include <coreutils/log.h>
#include <coreutils/format.h>

#include <string>

#ifdef EMSCRIPTEN
#include <SDL/SDL_image.h>
#else
#include "lodepng.h"
#endif
using namespace std;
using namespace utils;


namespace image {

#ifdef EMSCRIPTEN

bitmap load_png(const std::string &file_name) {
	SDL_Surface* s = IMG_Load(file_name.c_str());
	if(!s)
		throw image_exception(format("Could not load %s", file_name));
	return bitmap(s->w, s->h, reinterpret_cast<uint32_t*>(s->pixels));
}

#else

bitmap load_png(const std::string &file_name) {

	unsigned char* image;
	unsigned width, height, error;

	if((error = lodepng_decode32_file(&image, &width, &height, file_name.c_str())))
		throw image_exception(lodepng_error_text(error));

	bitmap bm(width, height, reinterpret_cast<uint32_t*>(image));

	free(image);

	return bm;
}

#endif

void save_png(bitmap bitmap, const std::string &path) {

	unsigned error = lodepng_encode32_file(path.c_str(), (const unsigned char*)bitmap.data(), bitmap.width(), bitmap.height());
	if(error != 0)
		throw image_exception(lodepng_error_text(error));
}


}
