#pragma once

// This must match the definitions in canvas_gen.frag!

struct Op {
	int opType;

	int pad0;
	int pad1;
	int pad2;

	float colour[4];

};

struct UniformData {
	Op ops[64];
	float baseColour[4]; // alpha component is unused

	// start of this image block in texture
	float offsetX;
	float offsetY;
	float width;
	float height;


	float uvX;
	float uvY;
	float uvWidth;
	float uvHeight;
};
