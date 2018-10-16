#include <Font.h>
#include <stdexcept>
#include <stb_rect_pack.h>
#include <iostream>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

using namespace std;

static FT_Library ft;

void initialise_font_loader()
{
	if(FT_Init_FreeType(&ft)) {
		throw runtime_error("Error initialising FreeType library");
	}
}

Font::Font(string && filepath, unsigned int fontSize)
{
	if(FT_New_Face(ft, filepath.c_str(), 0, &face)) {
		throw runtime_error("Error opening font file");
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
}

void Font::map_glyphs(unsigned int firstCharacter, Atlas & atlas, vector<stbrp_rect> & rects, unsigned int wh)
{
	stbrp_context context;
	vector<stbrp_node> nodes(128);
	stbrp_init_target (&context, wh, wh, nodes.data(), nodes.size());

	// TODO remove this and use glyph overloading
	unsigned int i = 0;
	if(!firstCharacter) {
		rects[0].w = 1;
		rects[0].h = 1;

		atlas.glyphs[0].advanceX = 64;
		atlas.glyphs[0].advanceY = 64;
		atlas.glyphs[0].bitmapTop = 1;
		atlas.glyphs[0].bitmapLeft = 0;

		i++;
	}

	for(; i < 128; i++) {
		FT_Load_Char(face, i + firstCharacter, FT_LOAD_RENDER); // TODO see if this works without render flag
		FT_GlyphSlot g = face->glyph;

		rects[i].w = g->bitmap.width;
		rects[i].h = g->bitmap.rows;

		atlas.glyphs[i].advanceX = g->advance.x;
		atlas.glyphs[i].advanceY = g->advance.y;
		atlas.glyphs[i].bitmapTop = g->bitmap_top;
		atlas.glyphs[i].bitmapLeft = g->bitmap_left;

	}

	if(!stbrp_pack_rects (&context, rects.data(), rects.size())) {
		throw runtime_error("Error creating font atlas in stbrp_pack_rects");
	}
}

// TODO: Allow overloading of characters with custom glyphs (for save icon, etc.)
void Font::load_font_atlas(unsigned int firstCharacter, Atlas & atlas)
{
	atlas.firstCharacter = firstCharacter;

	vector<stbrp_rect> rects(128);

	unsigned int textureSize = 128;
	atlas.largeTexture = false;
	try {
		map_glyphs(firstCharacter, atlas, rects, textureSize);
	}
	catch (const runtime_error& e) {
		clog << "Font atlas does not fit in 128x128px texture. Trying 256x256px." << endl;
		textureSize = 256;
		atlas.largeTexture = true;
		map_glyphs(firstCharacter, atlas, rects, textureSize);

	}

	vector<unsigned char> data(textureSize*textureSize);

	unsigned int i = 0;
	if(!firstCharacter) {
		// NULL character has 1 pixel of black
		data[(rects[0].y)*textureSize + rects[i].x] = 255;
	}

	for(; i < 128; i++) {
		FT_Load_Char(face, i + firstCharacter, FT_LOAD_RENDER);
		FT_GlyphSlot g = face->glyph;

		for(unsigned int y = 0; y < g->bitmap.rows; y++) {
			memcpy(&data[(rects[i].y + y)*textureSize + rects[i].x], &g->bitmap.buffer[y*g->bitmap.width], g->bitmap.width);
		}
		atlas.glyphs[i].x = rects[i].x;
		atlas.glyphs[i].y = rects[i].y;
		atlas.glyphs[i].w = rects[i].w;
		atlas.glyphs[i].h = rects[i].h;
	}

	glGenTextures(1, &atlas.textureId);
	glBindTexture(GL_TEXTURE_2D, atlas.textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureSize, textureSize, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());


}
