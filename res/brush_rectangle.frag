#version 140

uniform float opacity = 1.0;

in vec2 pass_coordinates;

out float outOpacity;

void main()
{
	float length = abs(max(pass_coordinates.x, pass_coordinates.y));
	if(length > 0.5) {
		discard;
	}
	else if (length > 0.45) {
		outOpacity = (1.0 - ((length-0.45) / 0.05)) * opacity;
	}
	else {
		outOpacity = opacity;
	}
}
