#version 140

struct LayerData {
	vec4 colour; // only if textureUnit = -1 or is a filter layer
	int type; // 0 = None (stop iterating over array), 1 = Layer, 2 = Layer group
	float textureIndex; // -1 = None, solid colour, 0+ = Index into array texture
	int blendModeOrFilter; // normal, add, sub, mul, div, etc. hsv adjust, greyscale, blur, etc.

	int firstChild; // first child layer (-1 if no children). Only if type == 2
	int next; // Nest layer in this layer group
	int parent;
	
	int imageFormat;
	int pad2;
}; // size = 48 bytes

layout (std140) uniform UniformData {
	LayerData layers[64];

	// start of this image block in texture
	float offsetX;
	float offsetY;
	float width;
	float height;

	int bottomLayer; // Layer at bottom of stack (Index into layers. Could be a group)

	int strokeLayer; // if not -1 then strokeImage should be sampled
	vec2 padding;
	vec4 strokeColour;
};

in vec2 coords;
in vec2 uv_coords;

out vec2 pass_coordinates;
out vec2 pass_canvas_coordinates;

void main()
{
	vec2 canvasFrameBufferCoordinates = vec2(offsetX + coords.x * width, offsetY + coords.y * height);
	gl_Position = vec4(canvasFrameBufferCoordinates.xy, 0, 1);

	// Convert to texture coordinates
	pass_canvas_coordinates = (canvasFrameBufferCoordinates + 1.0) * 0.5;

	pass_coordinates = uv_coords;
}
