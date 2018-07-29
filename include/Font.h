#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glad/glad.h>

class Font {
	FT_Face face;
public:

	Font(const char * filepath, unsigned int fontSize);

	struct FontGlyph {
		int advanceX; // Distance between start of this glyph and start of the next in 1/64 pixels
		int advanceY; // Also in 1/64 pixels. Only used if text is on multiple lines.
		int bitmapTop; // Height of glyph above baseline in pixels. Glyphs may go below the baseline - e.g. 'p', 'q'
		int bitmapLeft;

		// Texture coordinates
		unsigned int x;
		unsigned int y;
		unsigned int w;
		unsigned int h;
	};

	struct Atlas {
		// UTF-32 character
		unsigned int firstCharacter;
		GLuint textureId;
		FontGlyph glyphs[128];
	};

	void load_font_atlas(unsigned int firstCharacter, Atlas & atlas);

	Font(Font const&) = delete;

};


void initialise_font_loader();

