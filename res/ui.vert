#version 150 core

uniform mat4 matrix;

in uvec2 position;
in vec2 textureCoordinates;
in vec4 colour;

out vec4 pass_colour;
out vec2 pass_textureCoordinates;

void main()
{
	gl_Position = matrix * vec4(position.xy, 0, 1);
	pass_colour = colour;
	pass_textureCoordinates = textureCoordinates;
}
