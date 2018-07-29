#version 140

uniform sampler2D image;

in vec2 pass_texture_coordinates;

out vec4 outColour;

void main()
{
	outColour = vec4(texture(image, pass_texture_coordinates).rgb, 1.0);

	outColour = outColour;
}
