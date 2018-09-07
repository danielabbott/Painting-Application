#pragma once

#include <vector>
#include <cstdio>

enum class ImageFormat {
	FMT_RGBA, // Regular colour format
	FMT_RG, // For greyscale images - R is brightness and G is opacity
	FMT_R // For filter layers - R is opacity
};

// file should be opened in "rb" mode and the current position must be the start of the file
// wasPNG is set to true if the file was a PGN file and false if it was not
// wasPNG will only be set to false when an exception will be raised
// The caller is responsible for closing the file handle
std::vector<unsigned char> load_png(FILE * file, unsigned int & width, unsigned int & height, bool & wasPNG);


std::vector<unsigned char> load_webp(FILE * file, unsigned int & width, unsigned int & height, bool & wasWebp);
