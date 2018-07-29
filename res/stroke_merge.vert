#version 140

// Shader program for merging temporary stroke layer with active layer

uniform vec4 strokeLayerCoordinates;

in vec2 coords;
in vec2 uv_coords;

out vec2 image_block_uv_coords;
out vec2 pass_stroke_layer_uv_coords;

void main()
{
	image_block_uv_coords = vec2(uv_coords.x, 1.0 - uv_coords.y);
	pass_stroke_layer_uv_coords = strokeLayerCoordinates.xy + uv_coords*strokeLayerCoordinates.zw;
	gl_Position = vec4(coords.x * 2.0 - 1.0, coords.y * 2.0 - 1.0, 0.0, 1.0);
}
