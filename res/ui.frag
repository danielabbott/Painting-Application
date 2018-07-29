#version 150

uniform sampler2D font;

in vec4 pass_colour;
in vec2 pass_textureCoordinates;

out vec4 outColour;

void main()
{
	float opacity = texture(font, pass_textureCoordinates).r;
	outColour = vec4(pass_colour.rgb, pass_colour.a * opacity);
}
