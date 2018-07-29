#version 140

uniform mat4 matrix;

// From 0 to 1
uniform float height = 1.0;

in vec2 coords;

out vec2 pass_coordinates;

void main()
{
	vec2 c = vec2(coords.x, coords.y * height);

	gl_Position = matrix * vec4(c, 0, 1);
	pass_coordinates = c;


}
