#include <Font.h>
#include <stdexcept>
#include <stb_rect_pack.h>

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

Font::Font(const char * filepath, unsigned int fontSize)
{
	if(FT_New_Face(ft, filepath, 0, &face)) {
		throw runtime_error("Error opening font file");
	}

	FT_Set_Pixel_Sizes(face, 0, fontSize);
}

// TODO: Allow overloading of characters with custom glyphs (for save icon, etc.)
void Font::load_font_atlas(unsigned int firstCharacter, Atlas & atlas)
{
	atlas.firstCharacter = firstCharacter;

	stbrp_context context;
	stbrp_node * nodes = new stbrp_node[128 * sizeof(stbrp_node)];
	stbrp_init_target (&context, 128, 128, nodes, 128);


	stbrp_rect * rects = new stbrp_rect[128]();

	// TODO remove this and use glyph overloading
	unsigned int i = 0;
	if(!firstCharacter) {
		rects[0].w = 1;
		rects[0].h = 1;

		atlas.glyphs[0].advanceX = 1;
		atlas.glyphs[0].advanceY = 0;
		atlas.glyphs[0].bitmapTop = 0;
		atlas.glyphs[0].bitmapLeft = 0;

		i++;
	}

	for(; i < 128; i++) {
		FT_Load_Char(face, i + firstCharacter, FT_LOAD_RENDER); // TODO see if this works with render flag
		FT_GlyphSlot g = face->glyph;

		rects[i].w = g->bitmap.width;
		rects[i].h = g->bitmap.rows;

		atlas.glyphs[i].advanceX = g->advance.x;
		atlas.glyphs[i].advanceY = g->advance.y;
		atlas.glyphs[i].bitmapTop = g->bitmap_top;
		atlas.glyphs[i].bitmapLeft = g->bitmap_left;

	}

	if(!stbrp_pack_rects (&context, rects, 128)) {
		throw runtime_error("Error creating font atlas in stbrp_pack_rects");
	}

	unsigned char * data = new unsigned char[128*128];

	i = 0;
	if(!firstCharacter) {
		// NULL character has 1 pixel of black
		data[(rects[0].y)*128 + rects[i].x] = 255;
	}

	for(; i < 128; i++) {
		FT_Load_Char(face, i + firstCharacter, FT_LOAD_RENDER);
		FT_GlyphSlot g = face->glyph;

		for(unsigned int y = 0; y < g->bitmap.rows; y++) {
			memcpy(&data[(rects[i].y + y)*128 + rects[i].x], &g->bitmap.buffer[y*g->bitmap.width], g->bitmap.width);
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, data);


	delete[] nodes;
	delete[] data;


}
