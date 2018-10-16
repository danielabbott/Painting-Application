#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glad/glad.h>
#include <vector>
#include <string>

struct stbrp_rect;

class Font {
	FT_Face face;

public:

	Font(std::string && filepath, unsigned int fontSize);

	struct FontGlyph {
		int advanceX; // Distance between start of this glyph and start of the next in 1/64 pixels
		int advanceY; // Also in 1/64 pixels. Only used if text is on multiple lines.
		int bitmapTop; // Height of glyph above baseline in pixels. Glyphs may go below the baseline - e.g. 'p', 'q'
		int bitmapLeft;

		// Texture coordinates
		unsigned int x;
		unsigned int y;
		// w,h, are also the size of the glpyh when rendered on-screen
		unsigned int w;
		unsigned int h;
	};

	struct Atlas {
		// UTF-32 character
		unsigned int firstCharacter;
		GLuint textureId;
		FontGlyph glyphs[128];
		// Returns true if the font uses 256x256px textures, false for 128x128px textures
		bool largeTexture = false;
	};

	void load_font_atlas(unsigned int firstCharacter, Atlas & atlas);

	Font(Font const&) = delete;

private:

	void map_glyphs(unsigned int firstCharacter, Atlas & atlas, std::vector<stbrp_rect> & rects, unsigned int wh);
};


void initialise_font_loader();

