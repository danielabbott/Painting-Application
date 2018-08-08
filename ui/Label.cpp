#include <UI.h>
#include <Font.h>
#include <iostream>

using namespace std;

namespace UI {


Label::Label(std::string text_, LeftRightAlignment leftRightTextAlign_, TopBottomAlignment topBottomTextAlign_) : Label(text_, 0, 0, leftRightTextAlign_, topBottomTextAlign_) {}

extern Font::Atlas asciiAtlas;
Label::Label(string text_, unsigned int x, unsigned int y, LeftRightAlignment leftRightTextAlign_, TopBottomAlignment topBottomTextAlign_)
: Widget(x, y, 0, 0, leftRightTextAlign_, topBottomTextAlign_), widgetText(text_)
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
	width = textWidth; // textWidth includes 4 pixels of padding
	height = 14 + 2 + 2; // size 14 font plus 4 pixels of padding
}


}
