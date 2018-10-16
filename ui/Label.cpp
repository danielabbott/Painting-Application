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
	textWidth = 4*64; // 4px horizontal padding
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

extern unsigned int fontSize;
extern unsigned int fontVerticalPad;

void Label::getDimensions(unsigned int & width, unsigned int & height)
{
	width = textWidth; // textWidth already includes 4 pixels of padding
	height = fontSize + fontVerticalPad + fontVerticalPad; // includes fontVerticalPad*2 pixels of padding
}


}
