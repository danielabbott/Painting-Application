#include <UI.h>
#include <Font.h>
#include <iostream>

using namespace std;

namespace UI {

extern unsigned int fontSize;
extern unsigned int fontVerticalPad;
extern unsigned int fontHorizontalPad;


Label::Label(std::string text_, LeftRightAlignment leftRightTextAlign_, TopBottomAlignment topBottomTextAlign_)
: Label(text_, 0, 0, 0, 0, leftRightTextAlign_, topBottomTextAlign_) {}

Label::Label(std::string text_, unsigned int x, unsigned int y,LeftRightAlignment leftRightTextAlign_, TopBottomAlignment topBottomTextAlign_)
: Label(text_, x, y, 0, 0, leftRightTextAlign_, topBottomTextAlign_) {}

extern Font::Atlas asciiAtlas;
Label::Label(string text_, unsigned int x, unsigned int y, unsigned int w, unsigned int h, LeftRightAlignment leftRightTextAlign_, TopBottomAlignment topBottomTextAlign_)
: Widget(x, y, w, h, leftRightTextAlign_, topBottomTextAlign_), widgetText(text_)
{
	textWidth = fontHorizontalPad*2*64; // add padding to sides of widget.
	for(char c : widgetText) {
		if(c > 0) {
			Font::FontGlyph const& glyph = asciiAtlas.glyphs[(int)c];
			textWidth += glyph.advanceX;
		}
		else {
			textWidth += 640;
		}
	}
	textWidth = (textWidth + 63) / 64;
}

string const& Label::getText() { 
	return widgetText;
}

void Label::getDimensions(unsigned int & width, unsigned int & height)
{
	width = w ? w : textWidth; // textWidth already includes 4 pixels of padding
	height = h ? h : fontSize + fontVerticalPad + fontVerticalPad; // includes fontVerticalPad*2 pixels of padding
}


}
