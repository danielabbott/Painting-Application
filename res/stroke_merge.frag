// Shader program for merging temporary stroke layer with active layer


#ifdef FMT_RGBA
#define STROKE_COLOUR_TYPE vec4
#define STROKE_COLOUR strokeColour.rgb
#define IMAGE_BLOCK_TEXTURE_COMPONENTS rgba
#define DST_TYPE vec4
#define DST_ALPHA dst.a
#define DST_COLOUR dst.rgb
#define COLOUR_TYPE vec3
#endif

#ifdef FMT_RG
#define STROKE_COLOUR_TYPE vec2
#define STROKE_COLOUR strokeColour.r
#define IMAGE_BLOCK_TEXTURE_COMPONENTS rg
#define DST_TYPE vec2
#define DST_ALPHA dst.g
#define DST_COLOUR dst.r
#define COLOUR_TYPE float
#endif

#ifdef FMT_R
#define STROKE_COLOUR_TYPE float
#define DST_TYPE float
#endif

uniform sampler2D strokeImage;
uniform sampler2DArray imageBlock;

uniform STROKE_COLOUR_TYPE strokeColour;
uniform float textureArrayIndex = 0;

in vec2 image_block_uv_coords;
in vec2 pass_stroke_layer_uv_coords;

out DST_TYPE out_colour;

#ifdef FMT_R

// For alpha-only channels the merge operation is simpler as there is no colour information, only opacity

void main()
{
	float strokeOpacity = texture(strokeImage, pass_stroke_layer_uv_coords).r * strokeColour;
	float dst = texture(imageBlock, vec3(image_block_uv_coords, textureArrayIndex)).r;


	out_colour = strokeOpacity + dst * (1.0 - strokeOpacity);
}

#else

// RGBA and greyscale with alpha

void main()
{
	// Stroke alpha is premultiplied in the stroke texture
	float strokeOpacity = texture(strokeImage, pass_stroke_layer_uv_coords).r;
	DST_TYPE dst = texture(imageBlock, vec3(image_block_uv_coords, textureArrayIndex)).IMAGE_BLOCK_TEXTURE_COMPONENTS;

	float alpha = strokeOpacity + DST_ALPHA * (1.0 - strokeOpacity);
	COLOUR_TYPE colour;
	if(alpha <= 0) {
		colour = COLOUR_TYPE(0);
	}
	else {
		colour = (STROKE_COLOUR * strokeOpacity + DST_COLOUR*DST_ALPHA*(1.0 - strokeOpacity)) / alpha; 
	}

	out_colour = DST_TYPE(colour, alpha);
}

#endif

