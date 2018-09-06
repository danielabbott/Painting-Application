#version 140

// Pressure * activeColour.a
uniform float strokeAlpha = 1.0;
uniform float seed = 0.0;

in vec2 pass_coordinates;

out float outOpacity;


float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}
	
float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
  	vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

void main()
{
	// float length = max(abs(pass_coordinates.x), abs(pass_coordinates.y));
	float length = length(pass_coordinates);

	if(length > 0.5) {
		discard;
	}
	else {
		float noiseValue = noise(pass_coordinates*20.0 + vec2(seed, 0.0));

		if (length > 0.45) {
			outOpacity = (1.0 - ((length-0.45) * 20.0)) * noiseValue * strokeAlpha;
		}
		else {
			outOpacity = noiseValue * strokeAlpha;
		}
	}

}
