#define PNG_DEBUG 3 // TODO can this be removed?
#include <png.h>
#include <cstdint>
#include <istream>
#include <vector>
#include <string>
#include <stdexcept>
#include <Files.h>

using namespace std;

vector<unsigned char> load_png(FILE * file, unsigned int & width, unsigned int & height, bool & wasPNG)
{
	char header[8];

	if(fread(header, 8, 1, file) != 1) {
		throw runtime_error("IO error");
	}

	if (png_sig_cmp((png_const_bytep) header, 0, 8)) {
		wasPNG = false;
     	throw runtime_error("File is not PNG");
  	}

  	wasPNG = true;

  	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  	if(!png_ptr) {
  		throw runtime_error("libpng error in png_create_read_struct");
  	}

  	png_infop info_ptr = png_create_info_struct(png_ptr);
  	if(!info_ptr) {
  		throw runtime_error("libpng error in png_create_info_struct");
  	}

  	// Sets PNG error callback
  	if(setjmp(png_jmpbuf(png_ptr))) {
  		throw runtime_error("libpng error");
  	}

  	png_init_io(png_ptr, file);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if(color_type != PNG_COLOR_TYPE_RGBA || bit_depth != 8) {
    	// TODO: Support all possible images
    	throw runtime_error("Unsupported image type (must be 8 bits per component RGBA)");
    }

    vector<unsigned char> imageData(width*height*4);

    png_bytep * row_pointers = (png_bytep *)(new uintptr_t[height]);
    for (unsigned int y=0; y<height; y++) {
        row_pointers[y] = (png_byte*) &imageData.data()[y*width*4];
    }

    png_read_image(png_ptr, row_pointers);

    delete[] (uintptr_t*)row_pointers;

    return move(imageData);
}
