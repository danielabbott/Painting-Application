#version 140

// Pressure * activeColour.a
uniform float strokeAlpha = 1.0;

uniform float hardness = 0.45; // 0 to 0.5

in vec2 pass_coordinates;

out float outOpacity;

void main()
{
	float length = length(pass_coordinates);
	if(length > 0.5) {
		discard;
	}
	else if (length > hardness) {
		outOpacity = (1.0 - ((length-hardness) / (0.5 - hardness))) * strokeAlpha;
	}
	else {
		outOpacity = strokeAlpha;
	}
}
