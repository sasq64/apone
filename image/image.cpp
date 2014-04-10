#include "image.h"

#include <coreutils/log.h>
#include <coreutils/format.h>

#include <string>

#ifdef EMSCRIPTEN
#include <SDL/SDL_image.h>
#else
#include <png.h>
#endif
using namespace std;
using namespace utils;


namespace image {

#ifdef EMSCRIPTEN


bitmap load_png(const std::string &file_name) {
	SDL_Surface* s = IMG_Load(file_name.c_str());
	if(!s)
		throw image_exception(format("Could not load %s", file_name));
	return bitmap(s->w, s->h, s->pixels);
}

#else


bitmap load_png(const std::string &file_name) {
	unsigned char header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name.c_str(), "rb");
	if(!fp)
		throw image_exception(format("Could not open %s", file_name));
	int rc = fread(header, 1, 8, fp);
	if(rc < 8 || png_sig_cmp(header, 0, 8))
		throw image_exception(format("File %s is not a png file", file_name));

	// initialize stuff
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if(!png_ptr)
		throw image_exception("png_create_read_struct failed");

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
		throw image_exception("png_create_info_struct failed");

	if(setjmp(png_jmpbuf(png_ptr)))
		throw image_exception("Error during init_io");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	auto width = png_get_image_width(png_ptr, info_ptr);
	auto height = png_get_image_height(png_ptr, info_ptr);
	//auto color_type = png_get_color_type(png_ptr, info_ptr);
	auto bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	//if(info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);

	//auto number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	LOGD("%d %d %d", width, height, bit_depth);

	/* read file */
	if(setjmp(png_jmpbuf(png_ptr)))
		throw image_exception("Error during read_image");

	png_bytep *row_pointers = (png_bytep*)png_malloc(png_ptr, sizeof(void*) * height);//new png_bytep [height];//  (png_bytep*) malloc(sizeof(png_bytep) * height);

	int rowbytes;
	if(bit_depth == 16)
		rowbytes = width*8;
	else
		rowbytes = width*4;

	for(unsigned int y=0; y<height; y++)
		row_pointers[y] = (png_bytep)png_malloc(png_ptr, rowbytes);//new png_byte [rowbytes];

	LOGD("Reading image");
	png_read_image(png_ptr, row_pointers);
	//png_set_strip_16(png_ptr);

	LOGD("Creating bitmap");
	bitmap bm(width, height);

	for(unsigned int y=0; y<height; y++) {
		png_byte* row = row_pointers[y];
		memcpy(&bm[y*width], row, width*4);
	}

	for(unsigned int y=0; y<height; y++)
		png_free(png_ptr, row_pointers[y]);
	//delete[] row_pointers[y];
	//delete[] row_pointers;
	png_free(png_ptr, row_pointers);

	fclose(fp);

	return bm;
}

void save_png(bitmap bitmap, const std::string &path)
{
	FILE * fp;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	size_t x, y;
	png_byte ** row_pointers = NULL;
	/* "status" contains the return value of this function. At first
	   it is set to a value which means 'failure'. When the routine
	   has finished its work, it is set to a value which means
	   'success'. */
	//int status = -1;
	/* The following number is set by trial and error only. I cannot
	   see where it it is documented in the libpng manual.
	*/
	int pixel_size = 4;
	int depth = 8;
	
	fp = fopen (path.c_str(), "wb");
	if (! fp) {
		throw image_exception("failed");

	}

	png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
	   throw image_exception("failed");
	}
	
	info_ptr = png_create_info_struct (png_ptr);
	if (info_ptr == NULL) {
	   throw image_exception("failed");
	}
	
	/* Set up error handling. */

	if (setjmp (png_jmpbuf (png_ptr))) {
	   throw image_exception("failed");
	}
	
	/* Set image attributes. */

	png_set_IHDR (png_ptr,
				  info_ptr,
				  bitmap.width(),
				  bitmap.height(),
				  depth,
				  PNG_COLOR_TYPE_RGBA,
				  PNG_INTERLACE_NONE,
				  PNG_COMPRESSION_TYPE_DEFAULT,
				  PNG_FILTER_TYPE_DEFAULT);
	
	/* Initialize rows of PNG. */

	row_pointers = (png_byte**)png_malloc (png_ptr, bitmap.height() * sizeof (png_byte *));
	for (y = 0; y < bitmap.height(); ++y) {
		png_byte *row = (png_byte*)png_malloc (png_ptr, sizeof (uint8_t) * bitmap.width() * pixel_size);
		row_pointers[y] = row;
		for (x = 0; x < bitmap.width(); ++x) {
			//pixel_t * pixel = pixel_at (bitmap, x, y);
			uint32_t pixel = bitmap[x+y*bitmap.width()];
			*row++ = pixel&0xff;
			*row++ = (pixel>>8)&0xff;
			*row++ = (pixel>>16)&0xff;
			*row++ = (pixel>>24)&0xff;
		}
	}
	
	/* Write the image data to "fp". */

	png_init_io (png_ptr, fp);
	png_set_rows (png_ptr, info_ptr, row_pointers);
	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	/* The routine has successfully written the file, so we set
	   "status" to a value which indicates success. */
	
	for (y = 0; y < bitmap.height(); y++) {
		png_free (png_ptr, row_pointers[y]);
	}
	png_free (png_ptr, row_pointers);
	
	png_destroy_write_struct (&png_ptr, &info_ptr);
	fclose (fp);

}


#endif
}
