#version 140

uniform sampler2DArray rgbaTextures;
uniform sampler2DArray rgTextures;
uniform sampler2DArray rTextures;
uniform sampler2D strokeImage; // Temporary image containing the current stroke as it is drawn

struct Op {
	// 0 = END
	// See switch statement in main function for what each value does
	int opType;

	vec4 colour;

};

layout (std140) uniform UniformData {
	Op ops[64];
	vec4 baseColour; // alpha component is unused

	// start of this image block in texture
	float offsetX;
	float offsetY;
	float width;
	float height;

	
	float uvX;
	float uvY;
	float uvWidth;
	float uvHeight;
};


in vec2 pass_coordinates;
in vec2 pass_canvas_coordinates;

out vec4 outColour;

void main()
{
	outColour = vec4(baseColour.rgb, 1.0);
	for(int opIndex = 0;; opIndex++) {
		bool stop = false;

		switch(ops[opIndex].opType) {
			case 0:
				stop = true;
				break;

			case 1:
			{ // Normal blend colour (use vec4 colour)
				vec4 src = ops[opIndex].colour;
				outColour.rgb = mix(outColour.rgb, src.rgb, src.a);
				break;
			}

			case 2:
			{ // Normal blend colour (use colour.r as index into RGBA array texture)
				vec4 src = texture(rgbaTextures, vec3(pass_coordinates, ops[opIndex].colour.r));
				outColour.rgb = mix(outColour.rgb, src.rgb, src.a);
				break;
			}

			case 3:
			{ // Normal blend colour (use colour.r as index into RG array texture)
				vec4 src = texture(rgTextures, vec3(pass_coordinates, ops[opIndex].colour.r));
				outColour.rgb = mix(outColour.rgb, src.rrr, src.g);
				break;
			}

			case 4:
			{ // Apply stroke (uses strokeImage)
				float strokeOpacity = texture(strokeImage, pass_canvas_coordinates).r;
				outColour.rgb = mix(outColour.rgb, ops[opIndex].colour.rgb, strokeOpacity);
				break;
			}

			case 5:
			{ // Greyscale filter (using single-channel layer)
				float intensity = texture(rTextures, vec3(pass_coordinates, ops[opIndex].colour.r)).r;
				float grey = (outColour.rgb.r + outColour.rgb.g + outColour.rgb.b) / 3.0;
				outColour.rgb = mix(outColour.rgb, vec3(grey), intensity);
				break;
			}

			case 6:
			{ // Greyscale filter (full effect)
				float grey = (outColour.rgb.r + outColour.rgb.g + outColour.rgb.b) / 3.0;
				outColour.rgb =vec3(grey);
				break;
			}
		}


		if(opIndex == 63) {
			break;
		}

		if(stop) {
			break;
		}
	}
}
