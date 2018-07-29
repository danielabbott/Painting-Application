#version 140

uniform mat4 matrix;

in vec2 coords;
in vec2 uv_coords;

out vec2 pass_texture_coordinates;

void main()
{
	gl_Position = matrix * vec4(coords, 0, 1);
	pass_texture_coordinates = uv_coords;
}
