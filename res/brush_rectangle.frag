#version 140

uniform float strokeAlpha = 1.0;

in vec2 pass_coordinates;

out float outOpacity;

void main()
{
	float length = max(abs(pass_coordinates.x), abs(pass_coordinates.y));
	if(length > 0.5) {
		discard;
	}
	else if (length > 0.45) {
		outOpacity = (1.0 - ((length-0.45) * 20.0)) * strokeAlpha;
	}
	else {
		outOpacity = strokeAlpha;
	}
}
