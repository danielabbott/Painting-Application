#version 140

// The Uniform block must be identical to the one in canvas_gen.frag

struct Op {
	int opType;

	vec4 colour;

};

layout (std140) uniform UniformData {
	Op ops[64];
	vec4 baseColour; // alpha component is unused

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

in vec2 coords;
in vec2 uv_coords;

out vec2 pass_coordinates;
out vec2 pass_canvas_coordinates;

void main()
{
	vec2 canvasFrameBufferCoordinates = vec2(offsetX + coords.x * width, offsetY + coords.y * height);
	gl_Position = vec4(canvasFrameBufferCoordinates, 0, 1);

	// Convert to texture coordinates
	pass_canvas_coordinates = (canvasFrameBufferCoordinates + 1.0) * 0.5;

	pass_coordinates = vec2(uvX + uv_coords.x * uvWidth, (uvY + uv_coords.y * uvHeight));
}
