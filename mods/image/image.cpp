#include "image.h"

#include <coreutils/log.h>
#include <coreutils/format.h>

#include <string>

#ifdef EMSCRIPTEN
#include <SDL/SDL_image.h>
#else
#endif

#include "lodepng.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;
using namespace utils;

namespace image {


std::vector<bitmap> load_gifs(const std::string &filename)
{
	std::vector<bitmap> bitmaps;
	FILE *f;
	stbi__context s;

	if (!(f = stbi__fopen(filename.c_str(), "rb")))
		throw image_exception(format("Could not load %s", filename));

	stbi__start_file(&s, f);

	if (stbi__gif_test(&s))
	{
		int c;
		stbi__gif g;
		memset(&g, 0, sizeof(g));

		unsigned char *data;

		while((data = stbi__gif_load_next(&s, &g, &c, 4)))
		{
			if (data == (unsigned char*)&s)
			{
				data = 0;
				break;
			}
			
			bitmaps.emplace_back(g.w, g.h, (uint32_t*)data);
			//gr->delay = g.delay;
		}

		STBI_FREE(g.out);
	}
	fclose(f);
	return bitmaps;
}

#ifdef EMSCRIPTEN

bitmap load_png(const std::string &file_name) {
	SDL_Surface* s = IMG_Load(file_name.c_str());
	if(!s)
		throw image_exception(format("Could not load %s", file_name));
	return bitmap(s->w, s->h, reinterpret_cast<uint32_t*>(s->pixels));
}

#else

bitmap load_image(const std::string &file_name) {

	unsigned char* image;
	int width, height, comp;
	image = stbi_load(file_name.c_str(), &width, &height, &comp, 4);
	if(image == nullptr)
		throw image_exception(format("stb_image failed to load '%s'", file_name));
		
#if 0
	unsigned width, height, error, comp;
	if((error = lodepng_decode32_file(&image, &width, &height, file_name.c_str())))
		throw image_exception(lodepng_error_text(error));
#endif
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
