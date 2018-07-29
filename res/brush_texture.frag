#version 140

uniform float opacity = 1.0;
uniform sampler2D brushTexture;

in vec2 pass_coordinates;

out float outOpacity;

void main()
{
	outOpacity = texture(brushTexture, pass_coordinates);
}
