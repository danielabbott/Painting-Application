#pragma once

#include <Canvas.h>
#include <Layer.h>
#include <vector>
#include <ImageBlock.h>
#include "UniformData.h"

struct CanvasResources {
	FrameBuffer strokeLayer; // Canvas-sized texture that the current stroke is painted on (alpha-only)
	FrameBuffer canvasFrameBuffer;
	FrameBuffer imageBlockTempLayerRGBA;
	FrameBuffer imageBlockTempLayerRG;
	FrameBuffer imageBlockTempLayerR;

	std::vector<ImageBlock> imageBlocks;

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


	unsigned int canvasWidth = 7680;
	unsigned int canvasHeight = 4320;
	float canvasZoom = 0.08f;

	int canvasX = 0;
	int canvasY = 0;
	
	bool panning = false;
	unsigned int panningPrevCursorX, panningPrevCursorY;

	Layer * firstLayer = nullptr;
	Layer * activeLayer = nullptr;

	bool penDown = false;
	
	// Coordinates of previous cursor position (in canvas coordinate space)
	int prevCanvasCoordX;
	int prevCanvasCoordY;

	// RGBA
	// For greyscale with alpha, elements 0 and 3 are used
	// For alpha-only, index 3 is used
	float activeColour[4];

	// True if the canvas texture needs to be regenerated
	bool canvasDirty = true;
};
