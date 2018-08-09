#include <Canvas.h>
#include <Layer.h>
#include <vector>
#include <ImageBlock.h>
#include <Shader.h>
#include <cassert>
#include <UI.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Brush.h>
#include <Window.h>
#include <hwcaps.h>

using namespace std;


Layer * firstLayer = nullptr;
Layer * activeLayer = nullptr;

// 8K
static unsigned int canvasWidth = 7680;
static unsigned int canvasHeight = 4320;
static float canvasZoom = 0.08f;

// 4K
// static unsigned int canvasWidth = 3840;
// static unsigned int canvasHeight = 2160;
// static float canvasZoom = 0.16f;

// unsigned int canvasWidth = 1024;
// unsigned int canvasHeight = 1024;
// float canvasZoom = 0.6f;

// RGBA
// For greyscale with alpha, elements 0 and 3 are used
// For alpha-only, element 3 is used
static float activeColour[4];


// Position of centre of canvas on-screen (relative to top-left of UI canvas widget)
static int canvasX = -1;
static int canvasY = -1;

static vector<ImageBlock> imageBlocks;
static FrameBuffer strokeLayer; // Canvas-sized texture that the current stroke is painted on (alpha-only)
static FrameBuffer canvasFrameBuffer;
static FrameBuffer imageBlockTempLayerRGBA;
static FrameBuffer imageBlockTempLayerRG;
static FrameBuffer imageBlockTempLayerR;

static bool penDown = false;

static Brush testBrush;

Layer layers[2];
void create_layers()
{
	layers[0].type = Layer::LAYER;
	layers[0].name = "bottom layer";
	layers[1].type = Layer::LAYER;
	layers[1].name = "top layer";

	firstLayer = &layers[0];
	activeLayer = &layers[1];
	layers[0].next = &layers[1];

	activeColour[0] = 1;
	activeColour[1] = 0;
	activeColour[2] = 0;
	activeColour[3] = 0.95f;
}

Layer * get_first_layer()
{
	return firstLayer;
}

void set_active_layer(Layer * layer)
{
	// TODO have layer groups selectable just don't allow drawing on them
	assert(layer->type == Layer::LAYER);
	activeLayer = layer;
}

Layer * get_active_layer()
{
	return activeLayer;
}

// This must match the definitions in canvas_gen.frag!

struct Op {
	int opType;

	int pad0;
	int pad1;
	int pad2;

	float colour[4];

};

struct UniformData {
	Op ops[64];
	float baseColour[4]; // alpha component is unused

	// start of this image block in texture
	float offsetX;
	float offsetY;
	float width;
	float height;

	float strokeColour[4];
};

UniformData uniformData;

static GLuint shaderProgram = 0;
static GLuint ubo;

static GLuint strokeMergeShaderProgramRGBA;
static GLint  strokeMergeCoordsLocationRGBA;
static GLint  strokeMergeColourLocationRGBA;
static GLint  strokeMergeIndexLocationRGBA;

static GLuint strokeMergeShaderProgramRG;
static GLint  strokeMergeCoordsLocationRG;
static GLint  strokeMergeColourLocationRG;
static GLint  strokeMergeIndexLocationRG;

static GLuint strokeMergeShaderProgramR;
static GLint  strokeMergeCoordsLocationR;
static GLint  strokeMergeColourLocationR;
static GLint  strokeMergeIndexLocationR;

static GLuint vaoId, vboId;

static GLuint brushVaoId, brushVboId;
GLuint get_brush_vao() { return brushVaoId; }

static GLint canvasTextureShaderProgram = 0;
static GLint canvasTextureMatrixLocation;

// True if the canvas texture needs to be regenerated
static bool canvasDirty = true;

static ImageBlock * get_image_block_at(unsigned int x, unsigned int y)
{
	for(ImageBlock & block : imageBlocks) {
		if(x >= block.getX() && x < block.getX() + image_block_size()
			&& y >= block.getY() && y < block.getY() + image_block_size()) {
			return &block;
		}
	}

	return nullptr;
}

static inline void create_canvas_generation_shader_program()
{
	GLuint vsId = load_shader("res/canvas_gen.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/canvas_gen.frag", GL_FRAGMENT_SHADER);
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsId);
	glAttachShader(shaderProgram, fsId);
	link_shader_program(shaderProgram);
	glDeleteShader(vsId);
	glDeleteShader(fsId);

	GLuint uniBlockIndex = glGetUniformBlockIndex(shaderProgram, "UniformData");
	if(uniBlockIndex == GL_INVALID_INDEX) throw runtime_error("Canvas shader does not define uniform block 'UniformData'");
	glUniformBlockBinding(shaderProgram, uniBlockIndex, 0);

	bind_shader_program(shaderProgram);
	GLint uniLoc = glGetUniformLocation(shaderProgram, "rgbaTextures");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2DArray rgbaTextures");
	glUniform1i(uniLoc, 0);
	uniLoc = glGetUniformLocation(shaderProgram, "rgTextures");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2DArray rgTextures");
	glUniform1i(uniLoc, 1);
	uniLoc = glGetUniformLocation(shaderProgram, "rTextures");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2DArray rTextures");
	glUniform1i(uniLoc, 2);
	uniLoc = glGetUniformLocation(shaderProgram, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 3);

	validate_shader_program(shaderProgram);
}

static inline void create_simple_texture_shader_program()
{
	GLuint vsId = load_shader("res/simple_texture.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/simple_texture.frag", GL_FRAGMENT_SHADER);
	canvasTextureShaderProgram = glCreateProgram();
	glAttachShader(canvasTextureShaderProgram, vsId);
	glAttachShader(canvasTextureShaderProgram, fsId);
	link_shader_program(canvasTextureShaderProgram);
	glDeleteShader(vsId);
	glDeleteShader(fsId);
	canvasTextureMatrixLocation = glGetUniformLocation(canvasTextureShaderProgram, "matrix");
	if(canvasTextureMatrixLocation == -1) throw runtime_error("res/simple_texture.vert does not define uniform mat4 matrix");


	bind_shader_program(canvasTextureShaderProgram);
	
	GLint uniLoc = glGetUniformLocation(canvasTextureShaderProgram, "image");
	if(uniLoc == -1) throw runtime_error("res/simple_texture.frag does not define uniform sampler2D image");
	glUniform1i(uniLoc, 0);

	validate_shader_program(canvasTextureShaderProgram);
}

static inline void create_stroke_merge_shader_program()
{
	GLuint vsId = load_shader("res/stroke_merge.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/stroke_merge.frag", GL_FRAGMENT_SHADER, "#version 140\n#define FMT_RGBA\n");
	strokeMergeShaderProgramRGBA = glCreateProgram();
	glAttachShader(strokeMergeShaderProgramRGBA, vsId);
	glAttachShader(strokeMergeShaderProgramRGBA, fsId);
	link_shader_program(strokeMergeShaderProgramRGBA);
	bind_shader_program(strokeMergeShaderProgramRGBA);

	glDeleteShader(fsId);

	strokeMergeCoordsLocationRGBA = glGetUniformLocation(strokeMergeShaderProgramRGBA, "strokeLayerCoordinates");
	if(strokeMergeCoordsLocationRGBA == -1) throw runtime_error("res/stroke_merge.vert does not define uniform vec4 strokeLayerCoordinates");
	strokeMergeColourLocationRGBA = glGetUniformLocation(strokeMergeShaderProgramRGBA, "strokeColour");
	if(strokeMergeColourLocationRGBA == -1) throw runtime_error("res/stroke_merge.frag does not define uniform vec4 strokeColour");

	GLint uniLoc = glGetUniformLocation(strokeMergeShaderProgramRGBA, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 0);

	uniLoc = glGetUniformLocation(strokeMergeShaderProgramRGBA, "imageBlock");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2DArray imageBlock");
	glUniform1i(uniLoc, 1);

	strokeMergeIndexLocationRGBA = glGetUniformLocation(strokeMergeShaderProgramRGBA, "textureArrayIndex");
	if(strokeMergeIndexLocationRGBA == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float textureArrayIndex");

	validate_shader_program(strokeMergeShaderProgramRGBA);

	//

	fsId = load_shader("res/stroke_merge.frag", GL_FRAGMENT_SHADER, "#version 140\n#define FMT_RG\n");
	strokeMergeShaderProgramRG = glCreateProgram();
	glAttachShader(strokeMergeShaderProgramRG, vsId);
	glAttachShader(strokeMergeShaderProgramRG, fsId);
	link_shader_program(strokeMergeShaderProgramRG);
	bind_shader_program(strokeMergeShaderProgramRG);

	glDeleteShader(fsId);

	strokeMergeCoordsLocationRG = glGetUniformLocation(strokeMergeShaderProgramRG, "strokeLayerCoordinates");
	if(strokeMergeCoordsLocationRG == -1) throw runtime_error("res/stroke_merge.vert does not define uniform vec4 strokeLayerCoordinates");
	strokeMergeColourLocationRG = glGetUniformLocation(strokeMergeShaderProgramRG, "strokeColour");
	if(strokeMergeColourLocationRG == -1) throw runtime_error("res/stroke_merge.frag does not define uniform vec2 strokeColour");

	uniLoc = glGetUniformLocation(strokeMergeShaderProgramRG, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 0);

	uniLoc = glGetUniformLocation(strokeMergeShaderProgramRG, "imageBlock");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2DArray imageBlock");
	glUniform1i(uniLoc, 1);

	strokeMergeIndexLocationRG = glGetUniformLocation(strokeMergeShaderProgramRG, "textureArrayIndex");
	if(strokeMergeIndexLocationRG == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float textureArrayIndex");

	validate_shader_program(strokeMergeShaderProgramRG);
	//

	fsId = load_shader("res/stroke_merge.frag", GL_FRAGMENT_SHADER, "#version 140\n#define FMT_R\n");
	strokeMergeShaderProgramR = glCreateProgram();
	glAttachShader(strokeMergeShaderProgramR, vsId);
	glAttachShader(strokeMergeShaderProgramR, fsId);
	link_shader_program(strokeMergeShaderProgramR);
	bind_shader_program(strokeMergeShaderProgramR);

	glDeleteShader(vsId);
	glDeleteShader(fsId);

	strokeMergeCoordsLocationR = glGetUniformLocation(strokeMergeShaderProgramR, "strokeLayerCoordinates");
	if(strokeMergeCoordsLocationR == -1) throw runtime_error("res/stroke_merge.vert does not define uniform vec4 strokeLayerCoordinates");
	strokeMergeColourLocationR = glGetUniformLocation(strokeMergeShaderProgramR, "strokeColour");
	if(strokeMergeColourLocationR == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float strokeColour");

	uniLoc = glGetUniformLocation(strokeMergeShaderProgramR, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 0);

	uniLoc = glGetUniformLocation(strokeMergeShaderProgramR, "imageBlock");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2DArray imageBlock");
	glUniform1i(uniLoc, 1);

	strokeMergeIndexLocationR = glGetUniformLocation(strokeMergeShaderProgramR, "textureArrayIndex");
	if(strokeMergeIndexLocationR == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float textureArrayIndex");

	validate_shader_program(strokeMergeShaderProgramR);
}

static inline void create_opengl_buffers()
{
	glGenVertexArrays(1, &vaoId);
	if(!vaoId) throw runtime_error("Error creating VAO");
	glBindVertexArray(vaoId);

	glGenBuffers(1, &vboId);
	if(!vboId) throw runtime_error("Error creating canvas vertex buffer");
	glBindBuffer(GL_ARRAY_BUFFER, vboId);

	const float vertices[12] = {
		0,0,
		1,0,
		1,1,

		1,1,
		0,1,
		0,0
	};

	const float texCoords[12] = {
		0,1,
		1,1,
		1,0,

		1,0,
		0,0,
		0,1
	};
	
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices + sizeof texCoords, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof vertices, sizeof texCoords, texCoords);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)sizeof vertices);
	glEnableVertexAttribArray(1);

	//

	glGenVertexArrays(1, &brushVaoId);
	if(!brushVaoId) throw runtime_error("Error creating brush VAO");
	glBindVertexArray(brushVaoId);

	glGenBuffers(1, &brushVboId);
	if(!brushVboId) throw runtime_error("Error creating brush vertex buffer");
	glBindBuffer(GL_ARRAY_BUFFER, brushVboId);

	const float vertices2[12] = {
		-0.5f,-0.5f,
		0.5f,-0.5f,
		0.5f,0.5f,

		0.5f,0.5f,
		-0.5f,0.5f,
		-0.5f,-0.5f
	};
	
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices2, vertices2, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


	// Uniform buffer

	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof uniformData, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static inline void create_opengl_images()
{
	if(canvasWidth > max_texture_size() || canvasHeight > max_texture_size()) {
		throw runtime_error("Canvas too big");
	}

	canvasFrameBuffer.create(FMT_RGBA, canvasWidth, canvasHeight);
	strokeLayer.create(FMT_R, canvasWidth, canvasHeight);
	strokeLayer.clear();
	imageBlockTempLayerRGBA.create(FMT_RGBA, image_block_size(), image_block_size());
	imageBlockTempLayerRG.create(FMT_RG, image_block_size(), image_block_size());
	imageBlockTempLayerR.create(FMT_R, image_block_size(), image_block_size());

	imageBlocks = vector<ImageBlock>(((canvasHeight + image_block_size() - 1) / image_block_size()) * ((canvasWidth + image_block_size() - 1) / image_block_size()));

	try {
		unsigned int i = 0;
		for(unsigned int y = 0; y < (canvasHeight + image_block_size() - 1) / image_block_size(); y++) {
			for(unsigned int x = 0; x < (canvasWidth + image_block_size() - 1) / image_block_size(); x++, i++) {
				imageBlocks[i] = ImageBlock(x*image_block_size(), y*image_block_size());
				imageBlocks[i].create();
			}	
		}
	} catch(const runtime_error & e) {
		clog << "No tablets detected" << endl;
	}

}

void initialise_canvas_display(unsigned int x, unsigned int y)
{
	canvasX = x;
	canvasY = y;


	// Shader programs

	create_stroke_merge_shader_program();

	create_canvas_generation_shader_program();

	testBrush.create("res/brush_circle.frag");

	create_simple_texture_shader_program();	

	// Vertex Buffers and uniform buffers

	create_opengl_buffers();


	// Create textures and framebuffers

	create_opengl_images();


}

void free_canvas_resources()
{
	assert(shaderProgram);
	glDeleteProgram(shaderProgram);
	shaderProgram = 0;
	glDeleteProgram(canvasTextureShaderProgram);

	canvasFrameBuffer.destroy();
	imageBlockTempLayerRGBA.destroy();
	imageBlockTempLayerRG.destroy();
	imageBlockTempLayerR.destroy();
	strokeLayer.destroy();
	for(ImageBlock & block : imageBlocks) {
		block.destroy();
	}
	imageBlocks.clear();

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vboId);
	glDeleteBuffers(1, &brushVboId);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &ubo);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &vaoId);
	glDeleteVertexArrays(1, &brushVaoId);
}

void Canvas::draw() {
	unsigned int windowWidth, windowHeight;
	UI::get_window_dimensions(windowWidth, windowHeight);

	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	glBindVertexArray(vaoId);
	if(canvasDirty) {
		if(penDown && activeLayer) {
			if(activeLayer->imageFormat == FMT_RGBA) {
				uniformData.strokeColour[0] = activeColour[0];
				uniformData.strokeColour[1] = activeColour[1];
				uniformData.strokeColour[2] = activeColour[2];
				uniformData.strokeColour[3] = activeColour[3];
			}
			else if(activeLayer->imageFormat == FMT_RG) {
				uniformData.strokeColour[0] = activeColour[0];
				uniformData.strokeColour[1] = activeColour[0];
				uniformData.strokeColour[2] = activeColour[0];
				uniformData.strokeColour[3] = activeColour[3];
			}
			else {
				uniformData.strokeColour[0] = 1;
				uniformData.strokeColour[1] = 1;
				uniformData.strokeColour[2] = 1;
				uniformData.strokeColour[3] = activeColour[3];
			}
		}

		bind_shader_program(shaderProgram);
		glDisable(GL_BLEND);
		canvasFrameBuffer.bindFrameBuffer();

		glActiveTexture(GL_TEXTURE3);
		strokeLayer.bindTexture();

		for(ImageBlock const& block : imageBlocks) {
			unsigned int rgbaIndex = 0;
			unsigned int rgIndex = 0;
			unsigned int rIndex = 0;

			assert(firstLayer);
			Layer * layer = firstLayer;
			vector<Op> ops;

			uniformData.baseColour[0] = uniformData.baseColour[1] = uniformData.baseColour[2] = uniformData.baseColour[3] = 1;

			while(1) {
				if(layer->type == Layer::LAYER) {
					const ImageBlock::LayerData * layerData = nullptr;
					if(layer->imageFormat == FMT_RGBA) {
						layerData = &block.layersRGBA[rgbaIndex++];
					}
					else if(layer->imageFormat == FMT_RG) {
						layerData = &block.layersRG[rgIndex++];
					}
					else {
						layerData = &block.layersR[rIndex++];
					}

					if(layerData->dataType == ImageBlock::LayerData::SOLID_COLOUR) {
						if((layerData->colour >> 24) == 0xff) {
							ops.clear();
							uniformData.baseColour[0] = (layerData->colour & 0xff) / 255.0f;
							uniformData.baseColour[1] = ((layerData->colour >> 8) & 0xff) / 255.0f;
							uniformData.baseColour[2] = ((layerData->colour >> 16) & 0xff) / 255.0f;
							uniformData.baseColour[3] = ((layerData->colour >> 24) & 0xff) / 255.0f;
						}
						else {
							Op op;
							op.opType = 1; // Overlay colour (normal blend mode)

							op.colour[0] = (layerData->colour & 0xff) / 255.0f;
							op.colour[1] = ((layerData->colour >> 8) & 0xff) / 255.0f;
							op.colour[2] = ((layerData->colour >> 16) & 0xff) / 255.0f;
							op.colour[3] = ((layerData->colour >> 24) & 0xff) / 255.0f;
							ops.push_back(op);
						}
					}
					else {
						Op op;
						op.opType = 2 + layer->imageFormat;
						op.colour[0] = layer->imageFormatSpecificIndex + 0.1f;
						ops.push_back(op);
					}

					if(layer == activeLayer && penDown && block.hasStrokeData) {
						// Stroke must be overlayed
						Op op;
						op.opType = 4;
						ops.push_back(op);
					}
				}


				if(layer->firstChild) {
					assert(layer->type == Layer::LAYER);
					layer = layer->firstChild;
				}
				else {
					if(layer->next) {
						layer = layer->next;
					}
					else if(layer->parent && layer->parent->next) {
						layer = layer->parent->next;
					}
					else {
						break;
					}
				}
			}
			Op op = {};
			ops.push_back(op);

			memcpy(uniformData.ops, ops.data(), (ops.size() > 64 ? 64 : ops.size()) * sizeof(Op));

			uniformData.offsetX = ((block.getX() / (float)canvasWidth) * 2.0f) - 1.0f;
			uniformData.offsetY = (((1.0f - ((block.getY() + image_block_size()) / (float)canvasHeight)) * 2.0f) - 1.0f);
			uniformData.width   = ((image_block_size() / (float)canvasWidth) * 2.0f);
			uniformData.height  = ((image_block_size() / (float)canvasHeight) * 2.0f);

			glActiveTexture(GL_TEXTURE0);

			if(block.arrayTextureRGBA.isCreated()) {
				block.arrayTextureRGBA.bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glActiveTexture(GL_TEXTURE1);

			if(block.arrayTextureRG.isCreated()) {
				block.arrayTextureRG.bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glActiveTexture(GL_TEXTURE2);

			if(block.arrayTextureR.isCreated()) {
				block.arrayTextureR.bind();
			}
			else {
				glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
			}

			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferData(GL_UNIFORM_BUFFER, sizeof uniformData, &uniformData, GL_DYNAMIC_DRAW);

			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}


	bind_default_framebuffer();
	bind_shader_program(canvasTextureShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	canvasFrameBuffer.bindTexture();

	glm::mat4 m = 
		glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3(
			(float)(uiCanvasX + canvasX),
			(float)(uiCanvasY + canvasY),
		0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3((float)canvasWidth * canvasZoom, (float)canvasHeight * canvasZoom, 1.0))
		* glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f))
	;

	glUniformMatrix4fv(canvasTextureMatrixLocation, 1, GL_FALSE, &m[0][0]);
	

	glDrawArrays(GL_TRIANGLES, 0, 6);

	canvasDirty = false;

}


// Coordinates of previous cursor position (in canvas coordinate space)
static int prevCanvasCoordX;
static int prevCanvasCoordY;

void Canvas::cursorCoordsToCanvasCoords(unsigned int cursorX, unsigned int cursorY, int & x, int & y)
{
	unsigned int uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight;
	getArea(uiCanvasX, uiCanvasY, uiCanvasWidth, uiCanvasHeight);

	float canvasOnscreenWidth = canvasWidth * canvasZoom;
	float canvasOnscreenHeight = canvasHeight * canvasZoom;

	x = ((((int)cursorX - (int)canvasX) / canvasOnscreenWidth) + 0.5f) * (int)canvasWidth;
	y = ((((int)cursorY - (int)canvasY) / canvasOnscreenHeight) + 0.5f) * (int)canvasHeight;
}

static bool panning = false;
static unsigned int panningPrevCursorX, panningPrevCursorY;

bool Canvas::onMouseButtonReleasedOutsideWidget(unsigned int button)
{
	return onMouseButtonReleased(button);
}

bool Canvas::onMouseButtonReleased(unsigned int button)
{
	if(button == 0) {
		clog << "Pen released" << endl;
		penDown = false;

		// Stylus was lifted up, merge the stroke layer with the active layer and clear the stroke layer

		if(activeLayer->imageFormat == FMT_RGBA) {
			bind_shader_program(strokeMergeShaderProgramRGBA);
			glUniform4f(strokeMergeColourLocationRGBA, activeColour[0], activeColour[1], activeColour[2], activeColour[3]);
			imageBlockTempLayerRGBA.bindFrameBuffer();
		}
		else if(activeLayer->imageFormat == FMT_RG) {
			bind_shader_program(strokeMergeShaderProgramRG);
			glUniform2f(strokeMergeColourLocationRG, activeColour[0], activeColour[3]);
			imageBlockTempLayerRG.bindFrameBuffer();
		}
		else {
			bind_shader_program(strokeMergeShaderProgramR);
			glUniform1f(strokeMergeColourLocationR, activeColour[3]);
			imageBlockTempLayerR.bindFrameBuffer();
		}

		glActiveTexture(GL_TEXTURE0);
		strokeLayer.bindTexture();
		glDisable(GL_BLEND);
		glBindVertexArray(vaoId);

		glActiveTexture(GL_TEXTURE1);
		for(ImageBlock & block : imageBlocks) {
			if(block.hasStrokeData) {
				block.bindTexture(activeLayer);

				float strokeImageX = (block.getX() / (float)canvasWidth);
				float strokeImageY = 1.0f - (block.getY() + image_block_size()) / (float)canvasHeight;
				float strokeImageWidth  = image_block_size() / (float)canvasWidth;
				float strokeImageHeight = image_block_size() / (float)canvasHeight;

				glUniform4f(strokeMergeCoordsLocationRGBA, strokeImageX, strokeImageY, strokeImageWidth, strokeImageHeight);

				if(activeLayer->imageFormat == FMT_RGBA) {
					glUniform1f(strokeMergeIndexLocationRGBA, block.indexOf(activeLayer));
				}
				else if(activeLayer->imageFormat == FMT_RG) {
					glUniform1f(strokeMergeIndexLocationRG, block.indexOf(activeLayer));
				}
				else {
					glUniform1f(strokeMergeIndexLocationR, block.indexOf(activeLayer));
				}

				glDrawArrays(GL_TRIANGLES, 0, 6);

				block.copyTo(activeLayer);

				block.hasStrokeData = false;
			}
		}

		strokeLayer.clear();
	}
	else if (button == 2) {
		// Scroll wheel

		panning = false;
	}
	return true;
}

bool Canvas::onClicked(unsigned int button, unsigned int x, unsigned int y)
{
	if(button == 0) {
		clog << "Pen pressed" << endl;
		penDown = true;

		cursorCoordsToCanvasCoords(x, y, prevCanvasCoordX, prevCanvasCoordY);
	}
	else if (button == 2) {
		// Scroll wheel

		panning = true;
		panningPrevCursorX = x;
		panningPrevCursorY = y;
	}
	return false;
}

static void drawStroke(int canvasXcoord, int canvasYcoord, float pressure, unsigned int size)
{
	glm::mat4 m = 
		glm::ortho(0.0f, (float)canvasWidth, (float)canvasHeight, 0.0f)
		* glm::translate(glm::mat4(1.0f), glm::vec3((float)canvasXcoord, (float)canvasYcoord, 0.0f))
		* glm::scale(glm::mat4(1.0f), glm::vec3((float)size, (float)size, 1.0f))
	;

	glUniformMatrix4fv(testBrush.matrixUniformLocation, 1, GL_FALSE, &m[0][0]);
	glUniform1f(testBrush.opacityUniformLocation, pressure);

	glDrawArrays(GL_TRIANGLES, 0, 6);


	// Set flag in appropriate image block(s)

	ImageBlock * block;

	for(int y = canvasYcoord - (int)size/2; y < canvasYcoord + (int)size/2; y += image_block_size()) {
		for(int x = canvasXcoord - (int)size/2; x < canvasXcoord + (int)size/2; x += image_block_size()) {
			if((block = get_image_block_at(x, y))) block->hasStrokeData = true;
		}
		if((block = get_image_block_at(canvasXcoord+size/2, y))) block->hasStrokeData = true;
	}

	for(int x = canvasXcoord - (int)size/2; x < canvasXcoord + (int)size/2; x += image_block_size()) {
		if((block = get_image_block_at(x, canvasYcoord+size/2))) block->hasStrokeData = true;
	}
	if((block = get_image_block_at(canvasXcoord+size/2, canvasYcoord+size/2))) block->hasStrokeData = true;
}

bool useMouse = false;

void set_canvas_input_device(bool mouse)
{
	useMouse = mouse;
}

bool Canvas::onMouseMoved(unsigned int cursorX, unsigned int cursorY, float pressure)
{
	if(panning) {
		canvasX += (int)cursorX - (int)panningPrevCursorX;
		canvasY += (int)cursorY - (int)panningPrevCursorY;

		panningPrevCursorX = cursorX;
		panningPrevCursorY = cursorY;

		return true;
	}
	if(!penDown) {
		return false;
	}

	if(useMouse || !tablet_detected()) {
		pressure = 1;
	}
	else {
		if(pressure < 0) {
			// Ignore mouse input (tablet events may be duplicated as mouse events)
			return false;
		}
	}

	unsigned int size = 100;


	bind_shader_program(testBrush.shaderProgram);


	glEnable(GL_BLEND);

	// if a stroke overlaps itself it will never make itself lighter
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_MAX);

	strokeLayer.bindFrameBuffer();
	glBindVertexArray(brushVaoId);

	int canvasXcoord;
	int canvasYcoord;
	cursorCoordsToCanvasCoords(cursorX, cursorY, canvasXcoord, canvasYcoord);


	int diffX = canvasXcoord - prevCanvasCoordX;
	int diffY = canvasYcoord - prevCanvasCoordY;

	float distance = sqrt(diffX*diffX + diffY*diffY);
	float increment = size / 5 / distance;

	for(float mul = increment; mul < 1.0f; mul += increment) {
		drawStroke(prevCanvasCoordX + (int)(diffX * mul), prevCanvasCoordY + (int)(diffY * mul), pressure, size);
	}

	drawStroke(canvasXcoord, canvasYcoord, pressure, size);

	prevCanvasCoordX = canvasXcoord;
	prevCanvasCoordY = canvasYcoord;


	glBlendEquation(GL_FUNC_ADD);

	canvasDirty = true;
	return true;

}

bool Canvas::onScroll(unsigned int x, unsigned int y, int direction)
{
	(void)x;
	(void)y;
	
	if(direction > 0) {
		canvasZoom *= direction*2;
	}
	else {
		canvasZoom /= -direction*2;
	}

	float smallestZoom = 8 / (float)min(canvasWidth, canvasHeight);

	if(canvasZoom < smallestZoom) {
		canvasZoom = smallestZoom;
	}

	if(canvasZoom > 30) {
		canvasZoom = 30;
	}

	return true;
}

void testfunc_clear_layer2()
{
	for(ImageBlock & block : imageBlocks) {
		block.fillLayer(firstLayer->next, 0);
	}
	canvasDirty = true;
}
