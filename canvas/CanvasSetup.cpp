#include <Canvas.h>
#include <Layer.h>
#include <vector>
#include <ImageBlock.h>
#include <Shader.h>
#include <cassert>
#include <Brush.h>
#include <hwcaps.h>
#include "CanvasState.h"

using namespace std;

extern CanvasResources canvasResources;

extern Brush testBrush;

static bool globalCanvasResourcesAllocated = false;

static inline void create_canvas_generation_shader_program()
{
	GLuint vsId = load_shader("res/canvas_gen.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/canvas_gen.frag", GL_FRAGMENT_SHADER);
	canvasResources.shaderProgram = glCreateProgram();
	glAttachShader(canvasResources.shaderProgram, vsId);
	glAttachShader(canvasResources.shaderProgram, fsId);
	link_shader_program(canvasResources.shaderProgram);
	glDeleteShader(vsId);
	glDeleteShader(fsId);

	GLuint uniBlockIndex = glGetUniformBlockIndex(canvasResources.shaderProgram, "UniformData");
	if(uniBlockIndex == GL_INVALID_INDEX) throw runtime_error("Canvas shader does not define uniform block 'UniformData'");
	glUniformBlockBinding(canvasResources.shaderProgram, uniBlockIndex, 1);

	bind_shader_program(canvasResources.shaderProgram);
	GLint uniLoc = glGetUniformLocation(canvasResources.shaderProgram, "rgbaTextures");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2DArray rgbaTextures");
	glUniform1i(uniLoc, 0);
	uniLoc = glGetUniformLocation(canvasResources.shaderProgram, "rgTextures");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2DArray rgTextures");
	glUniform1i(uniLoc, 1);
	uniLoc = glGetUniformLocation(canvasResources.shaderProgram, "rTextures");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2DArray rTextures");
	glUniform1i(uniLoc, 2);
	uniLoc = glGetUniformLocation(canvasResources.shaderProgram, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/canvas_gen.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 3);

	validate_shader_program(canvasResources.shaderProgram);
}

static inline void create_simple_texture_shader_program()
{
	GLuint vsId = load_shader("res/simple_texture.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/simple_texture.frag", GL_FRAGMENT_SHADER);
	canvasResources.canvasTextureShaderProgram = glCreateProgram();
	glAttachShader(canvasResources.canvasTextureShaderProgram, vsId);
	glAttachShader(canvasResources.canvasTextureShaderProgram, fsId);
	link_shader_program(canvasResources.canvasTextureShaderProgram);
	glDeleteShader(vsId);
	glDeleteShader(fsId);
	canvasResources.canvasTextureMatrixLocation = glGetUniformLocation(canvasResources.canvasTextureShaderProgram, "matrix");
	if(canvasResources.canvasTextureMatrixLocation == -1) throw runtime_error("res/simple_texture.vert does not define uniform mat4 matrix");


	bind_shader_program(canvasResources.canvasTextureShaderProgram);
	
	GLint uniLoc = glGetUniformLocation(canvasResources.canvasTextureShaderProgram, "image");
	if(uniLoc == -1) throw runtime_error("res/simple_texture.frag does not define uniform sampler2D image");
	glUniform1i(uniLoc, 0);

	validate_shader_program(canvasResources.canvasTextureShaderProgram);
}

static inline void create_stroke_merge_shader_program()
{
	GLuint vsId = load_shader("res/stroke_merge.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/stroke_merge.frag", GL_FRAGMENT_SHADER, "#version 140\n#define FMT_RGBA\n");
	canvasResources.strokeMergeShaderProgramRGBA = glCreateProgram();
	glAttachShader(canvasResources.strokeMergeShaderProgramRGBA, vsId);
	glAttachShader(canvasResources.strokeMergeShaderProgramRGBA, fsId);
	link_shader_program(canvasResources.strokeMergeShaderProgramRGBA);
	bind_shader_program(canvasResources.strokeMergeShaderProgramRGBA);

	glDeleteShader(fsId);

	canvasResources.strokeMergeCoordsLocationRGBA = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRGBA, "strokeLayerCoordinates");
	if(canvasResources.strokeMergeCoordsLocationRGBA == -1) throw runtime_error("res/stroke_merge.vert does not define uniform vec4 strokeLayerCoordinates");
	canvasResources.strokeMergeColourLocationRGBA = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRGBA, "strokeColour");
	if(canvasResources.strokeMergeColourLocationRGBA == -1) throw runtime_error("res/stroke_merge.frag does not define uniform vec4 strokeColour");

	GLint uniLoc = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRGBA, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 0);

	uniLoc = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRGBA, "imageBlock");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2DArray imageBlock");
	glUniform1i(uniLoc, 1);

	canvasResources.strokeMergeIndexLocationRGBA = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRGBA, "textureArrayIndex");
	if(canvasResources.strokeMergeIndexLocationRGBA == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float textureArrayIndex");

	validate_shader_program(canvasResources.strokeMergeShaderProgramRGBA);

	//

	fsId = load_shader("res/stroke_merge.frag", GL_FRAGMENT_SHADER, "#version 140\n#define FMT_RG\n");
	canvasResources.strokeMergeShaderProgramRG = glCreateProgram();
	glAttachShader(canvasResources.strokeMergeShaderProgramRG, vsId);
	glAttachShader(canvasResources.strokeMergeShaderProgramRG, fsId);
	link_shader_program(canvasResources.strokeMergeShaderProgramRG);
	bind_shader_program(canvasResources.strokeMergeShaderProgramRG);

	glDeleteShader(fsId);

	canvasResources.strokeMergeCoordsLocationRG = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRG, "strokeLayerCoordinates");
	if(canvasResources.strokeMergeCoordsLocationRG == -1) throw runtime_error("res/stroke_merge.vert does not define uniform vec4 strokeLayerCoordinates");
	canvasResources.strokeMergeColourLocationRG = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRG, "strokeColour");
	if(canvasResources.strokeMergeColourLocationRG == -1) throw runtime_error("res/stroke_merge.frag does not define uniform vec2 strokeColour");

	uniLoc = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRG, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 0);

	uniLoc = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRG, "imageBlock");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2DArray imageBlock");
	glUniform1i(uniLoc, 1);

	canvasResources.strokeMergeIndexLocationRG = glGetUniformLocation(canvasResources.strokeMergeShaderProgramRG, "textureArrayIndex");
	if(canvasResources.strokeMergeIndexLocationRG == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float textureArrayIndex");

	validate_shader_program(canvasResources.strokeMergeShaderProgramRG);
	//

	fsId = load_shader("res/stroke_merge.frag", GL_FRAGMENT_SHADER, "#version 140\n#define FMT_R\n");
	canvasResources.strokeMergeShaderProgramR = glCreateProgram();
	glAttachShader(canvasResources.strokeMergeShaderProgramR, vsId);
	glAttachShader(canvasResources.strokeMergeShaderProgramR, fsId);
	link_shader_program(canvasResources.strokeMergeShaderProgramR);
	bind_shader_program(canvasResources.strokeMergeShaderProgramR);

	glDeleteShader(vsId);
	glDeleteShader(fsId);

	canvasResources.strokeMergeCoordsLocationR = glGetUniformLocation(canvasResources.strokeMergeShaderProgramR, "strokeLayerCoordinates");
	if(canvasResources.strokeMergeCoordsLocationR == -1) throw runtime_error("res/stroke_merge.vert does not define uniform vec4 strokeLayerCoordinates");
	canvasResources.strokeMergeColourLocationR = glGetUniformLocation(canvasResources.strokeMergeShaderProgramR, "strokeColour");
	if(canvasResources.strokeMergeColourLocationR == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float strokeColour");

	uniLoc = glGetUniformLocation(canvasResources.strokeMergeShaderProgramR, "strokeImage");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2D strokeImage");
	glUniform1i(uniLoc, 0);

	uniLoc = glGetUniformLocation(canvasResources.strokeMergeShaderProgramR, "imageBlock");
	if(uniLoc == -1) throw runtime_error("res/stroke_merge.frag does not define uniform sampler2DArray imageBlock");
	glUniform1i(uniLoc, 1);

	canvasResources.strokeMergeIndexLocationR = glGetUniformLocation(canvasResources.strokeMergeShaderProgramR, "textureArrayIndex");
	if(canvasResources.strokeMergeIndexLocationR == -1) throw runtime_error("res/stroke_merge.frag does not define uniform float textureArrayIndex");

	validate_shader_program(canvasResources.strokeMergeShaderProgramR);
}

static inline void create_opengl_buffers()
{
	glGenVertexArrays(1, &canvasResources.vaoId);
	if(!canvasResources.vaoId) throw runtime_error("Error creating VAO");
	glBindVertexArray(canvasResources.vaoId);

	glGenBuffers(1, &canvasResources.vboId);
	if(!canvasResources.vboId) throw runtime_error("Error creating canvas vertex buffer");
	glBindBuffer(GL_ARRAY_BUFFER, canvasResources.vboId);

	// In window/canvas coordinates, hence the units are pixels
	const float vertices[12] = {
		0,0,
		1,0,
		1,1,

		1,1,
		0,1,
		0,0
	};

	const float texCoords[12] = {
		0,1, // 0,0 vertex coordinates => texture coordinates 0,1
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

	glGenVertexArrays(1, &canvasResources.brushVaoId);
	if(!canvasResources.brushVaoId) throw runtime_error("Error creating brush VAO");
	glBindVertexArray(canvasResources.brushVaoId);

	glGenBuffers(1, &canvasResources.brushVboId);
	if(!canvasResources.brushVboId) throw runtime_error("Error creating brush vertex buffer");
	glBindBuffer(GL_ARRAY_BUFFER, canvasResources.brushVboId);

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

	glGenBuffers(1, &canvasResources.ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, canvasResources.ubo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformData), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, canvasResources.ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static inline void create_opengl_images(unsigned int canvasWidth, unsigned int canvasHeight)
{

	canvasResources.strokeLayer = new FrameBuffer(ImageFormat::FMT_R, canvasWidth, canvasHeight);
	canvasResources.strokeLayer->clear();
	canvasResources.imageBlockTempLayerRGBA = new FrameBuffer(ImageFormat::FMT_RGBA, image_block_size(), image_block_size(), true);
	canvasResources.imageBlockTempLayerRG = new FrameBuffer(ImageFormat::FMT_RG, image_block_size(), image_block_size(), true);
	canvasResources.imageBlockTempLayerR = new FrameBuffer(ImageFormat::FMT_R, image_block_size(), image_block_size(), true);

}

Canvas::Canvas(unsigned int width, unsigned int height) 
: canvasWidth(width), canvasHeight(height)
{
	if(canvasWidth > max_texture_size() || canvasHeight > max_texture_size()) {
		throw runtime_error("Canvas too big (exceeds OpenGL limitations)");
	}

	if(!globalCanvasResourcesAllocated) {

		// Shader programs

		create_stroke_merge_shader_program();

		create_canvas_generation_shader_program();

		// testBrush.blendMode = Brush::BlendMode::ADD;
		// testBrush.create("res/brush_noise.frag");
		
		testBrush.create("res/brush_circle.frag");

		create_simple_texture_shader_program();	

		// Vertex Buffers and uniform buffers

		create_opengl_buffers();

		// Create textures and framebuffers

		create_opengl_images(canvasWidth, canvasHeight);

		globalCanvasResourcesAllocated = true;
	}

	canvasFrameBuffer = new FrameBuffer(ImageFormat::FMT_RGBA, canvasWidth, canvasHeight);



	// create test layers

	LayerPtr rootLayer = make_shared<Layer>();
	LayerPtr layer1 = make_shared<Layer>();
	LayerPtr layer2 = make_shared<Layer>();

	layer1->type = Layer::Type::LAYER;
	layer1->name = "bottom layer";
	layer2->type = Layer::Type::LAYER;
	layer2->name = "top layer";

	rootLayer->type = Layer::Type::LAYER_GROUP;

	firstLayer = layer1;
	activeLayer = layer2;

	layer1->next = layer2;
	layer2->prev = layer1;
	layer1->parent = rootLayer;
	layer2->parent = rootLayer;
	rootLayer->firstChild = layer1;

	canvasResources.activeColour[0] = 1;
	canvasResources.activeColour[1] = 0;
	canvasResources.activeColour[2] = 0;
	canvasResources.activeColour[3] = 1;

	// Create image blocks

	imageBlocks = deque<ImageBlock>();

	try {
		unsigned int i = 0;
		for(unsigned int y = 0; y < (canvasHeight + image_block_size() - 1) / image_block_size(); y++) {
			for(unsigned int x = 0; x < (canvasWidth + image_block_size() - 1) / image_block_size(); x++, i++) {
				imageBlocks.emplace_back(x*image_block_size(), y*image_block_size(), *this);
			}	
		}
	} catch(const runtime_error & e) {
		clog << "No tablets detected" << endl;
	}

	for(ImageBlock & block : imageBlocks) {
		block.dirtyRegion(block.getX(), block.getY(), image_block_size(), image_block_size());
		block.fillLayer(firstLayer, 0xffa0a0a0);
		block.fillLayer(activeLayer, 0x00ffffff);
	}
	canvasDirty = true;
}

void Canvas::freeCanvasResources()
{
	delete canvasFrameBuffer;
	imageBlocks.clear();
} 

void free_canvas_resources()
{
	assert(canvasResources.shaderProgram);
	glDeleteProgram(canvasResources.shaderProgram);
	canvasResources.shaderProgram = 0;
	glDeleteProgram(canvasResources.canvasTextureShaderProgram);

	delete canvasResources.imageBlockTempLayerRGBA;
	delete canvasResources.imageBlockTempLayerRG;
	delete canvasResources.imageBlockTempLayerR;
	delete canvasResources.strokeLayer;

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &canvasResources.vboId);
	glDeleteBuffers(1, &canvasResources.brushVboId);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glDeleteBuffers(1, &canvasResources.ubo);
	glBindVertexArray(0);

	glDeleteVertexArrays(1, &canvasResources.vaoId);
	glDeleteVertexArrays(1, &canvasResources.brushVaoId);
}
