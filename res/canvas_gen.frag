#version 140

uniform sampler2DArray rgbaTextures;
uniform sampler2DArray rgTextures;
uniform sampler2DArray rTextures;
uniform sampler2D strokeImage; // Temporary image containing the current stroke as it is drawn

struct LayerData {
	vec4 colour; // only if textureUnit = -1 or is a filter layer
	int type; // 0 = None (stop iterating over array), 1 = Layer, 2 = Layer group
	float textureIndex; // -1 = None, solid colour, 0+ = Index into array texture
	int blendModeOrFilter; // normal, add, sub, mul, div, etc. hsv adjust, greyscale, blur, etc.

	int firstChild; // first child layer (-1 if no children). Only if type == 2
	int next; // Nest layer in this layer group
	int parent;

	int imageFormat; // 0 = RGBA, 1 = greyscale and alpha, 2 = alpha
	int pad2;
}; // size = 48 bytes

layout (std140) uniform UniformData {
	LayerData layers[64];

	// start of this image block in texture
	float offsetX;
	float offsetY;
	float width;
	float height;

	int bottomLayer; // Layer at bottom of stack (Index into layers. Could be a group)

	int strokeLayer; // if not -1 then strokeImage should be sampled
	vec2 padding;
	vec4 strokeColour;
};


in vec2 pass_coordinates;
in vec2 pass_canvas_coordinates;

out vec4 outColour;

void main()
{
	vec3 finalColour = vec3(1.0);

	int currentLayer = bottomLayer;

	while(currentLayer != -1) {
		int type = layers[currentLayer].type;
		if(type == 1) {
			// Apply this layer's pixel to the final colour

			if(layers[currentLayer].textureIndex < -0.5) {
				vec4 src = layers[currentLayer].colour;
				finalColour = mix(finalColour, src.rgb, src.a);
			}
			else {
				if(layers[currentLayer].imageFormat == 0) {
					vec4 src = texture(rgbaTextures, vec3(pass_coordinates, layers[currentLayer].textureIndex));
					finalColour = mix(finalColour, src.rgb, src.a);
				}
				else if(layers[currentLayer].imageFormat == 1) {
					vec4 src = texture(rgTextures, vec3(pass_coordinates, layers[currentLayer].textureIndex)).rrrg;
					finalColour = mix(finalColour, src.rgb, src.a);
				}
				else {
					finalColour = texture(rTextures, vec3(pass_coordinates, layers[currentLayer].textureIndex)).rrr;
				}
			}


			if(strokeLayer == currentLayer) {
				// The user is drawing on this layer. 
				// Draw the brush stroke.

				float strokeOpacity = texture(strokeImage, pass_canvas_coordinates).r * strokeColour.a;
				finalColour = mix(finalColour, strokeColour.rgb, strokeOpacity);
			}



			// Got to the next layer (or go back up the layers)

			if(layers[currentLayer].next != -1) {
				currentLayer = layers[currentLayer].next;
			}
			else {
				currentLayer = layers[currentLayer].parent;

				if(currentLayer == -1) {
					break;
				}

				do {
					// Parent is a group (it has to be)
					// Move to the next layer to avoid infinitely looping

					if(layers[currentLayer].next != -1) {
						currentLayer = layers[currentLayer].next;
						break;
					}
					else {
						// There is no next layer, move up to the parent then.
						currentLayer = layers[currentLayer].parent;
					}
				} while (currentLayer != -1);
			}
		}
		else if(type == 2) {
			if(layers[currentLayer].firstChild != -1) {
				currentLayer = layers[currentLayer].firstChild;
			} else {
				if(layers[currentLayer].next != -1) {
					currentLayer = layers[currentLayer].next;
				}
				else {
					currentLayer = layers[currentLayer].parent;
				}
			}
		}
		else {
			break;
		}
	}

	outColour = vec4(finalColour, 1.0);
}
