#include <UI.h>
#include <Font.h>

using namespace std;

namespace UI {


Label::Label(std::string text_) : Label(text_, 0, 0) {}

extern Font::Atlas asciiAtlas;
Label::Label(string text_, unsigned int x, unsigned int y) : Widget(x, y), widgetText(text_)
{
	textWidth = 4*64;
	for(char c : widgetText) {
		if(c > 0) {
			Font::FontGlyph const& glyph = asciiAtlas.glyphs[(int)c];
			textWidth += glyph.advanceX;
		}
		else {
			textWidth += 640;
		}
	}
	textWidth /= 64;
}

std::string const& Label::getText() { 
	return widgetText;
}

void Label::getDimensions(unsigned int & width, unsigned int & height)
{
	width = textWidth;
	height = 14;
}


}
