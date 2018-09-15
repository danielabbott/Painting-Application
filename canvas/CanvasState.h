#pragma once

#include <Layer.h>
#include <deque>
#include <ImageBlock.h>
#include <FrameBuffer.h>
#include "UniformData.h"

struct CanvasResources {
	
	// Canvas-sized texture that the current stroke is painted on (alpha-only)
	// The same framebuffer is used for all open canvases
	// It's dimensions match the size of the largest canvas
	FrameBuffer * strokeLayer;

	FrameBuffer * imageBlockTempLayerRGBA;
	FrameBuffer * imageBlockTempLayerRG;
	FrameBuffer * imageBlockTempLayerR;

	GLuint shaderProgram;
	GLuint ubo;

	GLuint strokeMergeShaderProgramRGBA;
	GLint  strokeMergeCoordsLocationRGBA;
	GLint  strokeMergeColourLocationRGBA;
	GLint  strokeMergeIndexLocationRGBA;

	GLuint strokeMergeShaderProgramRG;
	GLint  strokeMergeCoordsLocationRG;
	GLint  strokeMergeColourLocationRG;
	GLint  strokeMergeIndexLocationRG;

	GLuint strokeMergeShaderProgramR;
	GLint  strokeMergeCoordsLocationR;
	GLint  strokeMergeColourLocationR;
	GLint  strokeMergeIndexLocationR;

	GLuint vaoId, vboId;

	GLuint brushVaoId, brushVboId;

	GLint canvasTextureShaderProgram;
	GLint canvasTextureMatrixLocation;

	UniformData uniformData;

	// RGBA
	// For greyscale with alpha, elements 0 and 3 are used
	// For alpha-only, index 3 is used
	// Active colour is independent of the active canvas
	float activeColour[4];

	
};
