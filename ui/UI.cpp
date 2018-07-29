#include <UI.h>
#include <Font.h>
#include <Shader.h>
#include <Window.h>
#include <stdexcept>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <FrameBuffer.h>

using namespace std;

namespace UI {

static bool globalDirtyFlag = true;

static Container * rootContainer = nullptr;
static Container * rootContainer2 = nullptr;

void set_root_container(Container * c) { rootContainer = c; rootContainer->bake(); }
void set_menu_overlay_root_container(Container * c) { rootContainer2 = c; rootContainer2->bake(); }

// Each index corresponds to a mouse button
Widget * widgetBeingClicked[3] = {};

void Container::bake() 
{
	assert(layoutManager == NONE);
	if(layoutManager == NONE) {
		actualWidth = w;
		actualHeight = h;
		actualX = x;
		actualY = y; // TODO: parent should set this..


		for(Widget * widget : widgets) {
			widget->actualX = actualX + widget->x;
			widget->actualY = actualY + widget->y;
			widget->actualWidth = widget->getWidth();
			widget->actualHeight = widget->getHeight();
		}
	}
	// TODO
	// else if(layoutManager == FLOW_ACROSS) {
	// 	actualWidth = width_;
	// 	actualHeight = height_;

	// 	unsigned int widgetX = 0;
	// 	for(Widget * widget : widgets) {
	// 		widget->actualX = actualX + widgetX;
	// 		widget->actualY = actualY;

	// 		unsigned int w = widget->getWidth();
	// 		unsigned int h = widget->getHeight();

	// 		widgetX += w;
	// 		widget->actualWidth = w;
	// 		widget->actualHeight = h;
	// 	}
	// }

	for(Widget * widget : widgets) {
		Container * container = dynamic_cast<Container *>(widget);
		if(container) {
			childContainers.push_back(container);
			container->bake();
		}
	}
}

struct Vertex {
	uint16_t x;
	uint16_t y;
	uint16_t textureX;
	uint16_t textureY;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t alpha;

	Vertex(uint16_t x_, uint16_t y_, uint16_t textureX_, uint16_t textureY_, uint8_t red_, uint8_t green_, uint8_t blue_, uint8_t alpha_)
	: x(x_), y(y_), textureX(textureX_), textureY(textureY_), red(red_), green(green_), blue(blue_), alpha(alpha_)
	{}
};

static unsigned int windowWidth = 300;
static unsigned int windowHeight = 200;

static Vertex * vertexData;
static unsigned int vertexDataNextIndex = 0;
static uint16_t * indexData;
static unsigned int indexDataNextIndex = 0;

static GLuint shaderProgram;
static GLuint vboId;
static GLuint iboId;
static GLuint vaoId;
static GLuint matrixUniformId;

static Font * font;
Font::Atlas asciiAtlas;

static bool windowDimensionsChanged = true;

void window_resized(unsigned int newWidth, unsigned int newHeight)
{
	windowWidth = newWidth;
	windowHeight = newHeight;
	globalDirtyFlag = true;
	windowDimensionsChanged = true;

	glViewport(0, 0, newWidth, newHeight);
}
void mouse_clicked(unsigned int button, unsigned int x, unsigned int y, bool buttonDown)
{
	if(button > 2) return;

	if(buttonDown) {
		Container::EventHandlerOutcome outcome = Container::NOTHING;
		if(rootContainer2 && rootContainer2->onClicked_(button, x, y, outcome)) {
			globalDirtyFlag = true;
		}

		if(outcome == Container::NOTHING) {
			if(rootContainer2) {
				rootContainer2 = nullptr;
				globalDirtyFlag = true;
			}

			if(rootContainer) {
				if(rootContainer->onClicked_(button, x, y, outcome)) {
					globalDirtyFlag = true;
				}
			}
		}
	}
	else {
		if(widgetBeingClicked[button]) {
			if(x_in_region(x, y, widgetBeingClicked[button]->actualX, widgetBeingClicked[button]->actualY, widgetBeingClicked[button]->actualWidth, widgetBeingClicked[button]->actualHeight)) {
				if(widgetBeingClicked[button]->onMouseButtonReleased(button)) {
					globalDirtyFlag = true;
				}
			}
			else {
				if(widgetBeingClicked[button]->onMouseButtonReleasedOutsideWidget(button)) {
					globalDirtyFlag = true;
				}
			}
			widgetBeingClicked[button] = nullptr;
		}
	}
}

void stylus_moved(unsigned int x, unsigned int y, float pressure)
{
	Container::EventHandlerOutcome outcome = Container::NOTHING;
	if(rootContainer2 && rootContainer2->onMouseMoved_(x, y, pressure, outcome)) {
		globalDirtyFlag = true;
	}

	if(rootContainer && outcome == Container::NOTHING) {
		if(rootContainer->onMouseMoved_(x, y, pressure, outcome)) {
			globalDirtyFlag = true;
		}
	}
}

void mouse_moved(unsigned int x, unsigned int y)
{
	Container::EventHandlerOutcome outcome = Container::NOTHING;
	if(rootContainer2 && rootContainer2->onMouseMoved_(x, y, -1, outcome)) {
		globalDirtyFlag = true;
	}

	if(rootContainer && outcome == Container::NOTHING) {
		if(rootContainer->onMouseMoved_(x, y, -1, outcome)) {
			globalDirtyFlag = true;
		}
	}
}

void scroll(unsigned int mouseX, unsigned int mouseY, int x, int y)
{
	(void)x;

	Container::EventHandlerOutcome outcome = Container::NOTHING;
	if(rootContainer2 && rootContainer2->onScroll_(mouseX, mouseY, y, outcome)) {
		globalDirtyFlag = true;
	}

	if(rootContainer && outcome == Container::NOTHING) {
		if(rootContainer->onScroll_(mouseX, mouseY, y, outcome)) {
			globalDirtyFlag = true;
		}
	}
}

void initialise_ui()
{

	assign_window_resize_callback(window_resized);
	assign_mouse_click_callback(mouse_clicked);
	assign_stylus_motion_callback(stylus_moved);
	assign_mouse_motion_callback(mouse_moved);
	assign_scroll_callback(scroll);

	// Shader program

	GLuint vsId = load_shader("res/ui.vert", GL_VERTEX_SHADER);
	GLuint fsId = load_shader("res/ui.frag", GL_FRAGMENT_SHADER);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vsId);
	glAttachShader(shaderProgram, fsId);

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "textureCoordinates");
	glBindAttribLocation(shaderProgram, 2, "colour");

	link_shader_program(shaderProgram);

	glDeleteShader(vsId);
	glDeleteShader(fsId);

	matrixUniformId = glGetUniformLocation(shaderProgram, "matrix");
	bind_shader_program(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "font"), 0);

	// VBO/VAO

	glGenVertexArrays(1, &vaoId);
	if(!vaoId) throw runtime_error("Error creating VAO");
	glBindVertexArray(vaoId);

	glGenBuffers(1, &vboId);
	if(!vboId) throw runtime_error("Error creating UI Vertex Buffer");

	glGenBuffers(1, &iboId);
	if(!iboId) throw runtime_error("Error creating UI Index Buffer");

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

	glVertexAttribIPointer(0, 2, GL_UNSIGNED_SHORT, sizeof(Vertex), (void *)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_SHORT, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, textureX));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, red));
	glEnableVertexAttribArray(2);

	// Font

	initialise_font_loader();
	font = new Font("res/Roboto-Regular.ttf", 12);
	font->load_font_atlas(0, asciiAtlas);


}

static void add_quad(uint16_t x, uint16_t y, uint16_t w, uint16_t h, 
	uint16_t texX, uint16_t texY, uint16_t texWidth, uint16_t texHeight,
	uint32_t colour)
{
	if(indexDataNextIndex > 24576-6 || vertexDataNextIndex > 16384-4) {
		throw exception();
	}

	Vertex v = Vertex(x, y, texX, texY, (uint8_t)colour, (uint8_t)(colour >> 8), (uint8_t)(colour >> 16), (uint8_t)(colour >> 24));

	indexData[indexDataNextIndex++] = vertexDataNextIndex;
	vertexData[vertexDataNextIndex++] = v;

	indexData[indexDataNextIndex++] = vertexDataNextIndex;
	v.x += w;
	v.textureX += texWidth;
	vertexData[vertexDataNextIndex++] = v;

	indexData[indexDataNextIndex++] = vertexDataNextIndex;
	indexData[indexDataNextIndex++] = vertexDataNextIndex;
	v.y += h;
	v.textureY += texHeight;
	vertexData[vertexDataNextIndex++] = v;

	indexData[indexDataNextIndex++] = vertexDataNextIndex;
	v.x = x;
	v.textureX = texX;
	vertexData[vertexDataNextIndex++] = v;

	indexData[indexDataNextIndex++] = vertexDataNextIndex-4;
}

void Container::draw(vector<Canvas *> & canvases)
{
	if(!active) {
		return;
	}

	Font::FontGlyph const& nullGlyph = asciiAtlas.glyphs[0];

	uint32_t colour = getBackGroundColour();
	if(this != rootContainer && colour & 0xff000000) {
		// Root container background colour is done through glClear
		add_quad(actualX, actualY, actualWidth, actualHeight, nullGlyph.x*256, nullGlyph.y*256, 0, 0, colour);
	}

	for(Widget * widget : widgets) {
		if(dynamic_cast<Container *>(widget)) {
			continue;
		}

		Canvas * canvas = dynamic_cast<Canvas *>(widget);
		if(canvas) {
			canvases.push_back(canvas);
		}

		colour = widget->getBackGroundColour();
		if(colour & 0xff000000) {
			add_quad(widget->actualX, widget->actualY, widget->actualWidth, widget->actualHeight, nullGlyph.x*256, nullGlyph.y*256, 0, 0, colour);
		}

		string const& text = widget->getText();
		if(text.size()) {
			unsigned int textColour = widget->getTextColour();
			unsigned int textX = (widget->actualX + 2) * 64;
			unsigned int textY = widget->actualY + 12;
			// TODO UTF-8 support
			for(char c : text) {
				if(c > 0/* && c < 128*/) {
					Font::FontGlyph const& glyph = asciiAtlas.glyphs[(int)c];
					add_quad(textX / 64 + glyph.bitmapLeft, textY - glyph.bitmapTop, glyph.w, glyph.h, glyph.x*256, glyph.y*256, glyph.w*256, glyph.h*256, textColour);
					textX += glyph.advanceX;
				}
				else {
					textX += 640;
				}
			}
		}
	}

	for(Container * c : childContainers) {
		c->draw(canvases);
	}
}

void Container::findCanvases(std::vector<Canvas *> & canvases)
{
	for(Widget * widget : widgets) {
		Canvas * canvas = dynamic_cast<Canvas *>(widget);
		if(canvas) {
			canvases.push_back(canvas);
		}		
	}

	for(Container * c : childContainers) {
		c->findCanvases(canvases);
	}
}

bool draw_ui(bool forceRedraw)
{
	if(globalDirtyFlag || forceRedraw || windowDimensionsChanged) {
		// Create vertex/index data (store data directly to OpenGL-mapped memory)

		bind_default_framebuffer();
		glClearColor(0.1f, 0.1f, 0.1f, 1);
		glClear(GL_COLOR_BUFFER_BIT);


		vector<Canvas *> canvases;

		if(globalDirtyFlag || windowDimensionsChanged) {
			// Buffers only need to be regenerated if something has changed
			// If the UI is being forced to redraw then just reuse the buffer as-is

			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, vboId);
			glBufferData(GL_ARRAY_BUFFER, 16384*sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24576*2, nullptr, GL_DYNAMIC_DRAW);

			vertexData = (Vertex *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			vertexDataNextIndex = 0;
			indexData = (uint16_t *) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
			indexDataNextIndex = 0;

			if(!vertexData || !indexData) {
				throw runtime_error("OpenGL Error in glMapBufferRange()");
			}

			try {
				if(rootContainer) {
					rootContainer->draw(canvases);
				}
				if(rootContainer2) {
					rootContainer2->draw(canvases);
				}
			} catch(exception & e) {
				static bool a = false;
				if(!a) {
					a = true;
					clog << "GUI is too complicated for data to fit in vertex/index buffers" << endl;
				}
			}

			assert(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
			assert(glUnmapBuffer(GL_ARRAY_BUFFER));
		}
		else {
			if(rootContainer) {
				rootContainer->findCanvases(canvases);
			}
			if(rootContainer2) {
				rootContainer2->findCanvases(canvases);
			}
		}

		// Now call the callback functions for the canvases
		for(Canvas * canvas : canvases) {
			canvas->draw();
		}

		// Draw the UI
		
		bind_default_framebuffer();

		bind_shader_program(shaderProgram);
		glBindVertexArray(vaoId);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(windowDimensionsChanged) {
			glm::mat4 matrix = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
			glUniformMatrix4fv(matrixUniformId, 1, GL_FALSE, &matrix[0][0]);
			windowDimensionsChanged = false;
		}

		glBindTexture(GL_TEXTURE_2D, asciiAtlas.textureId);
		glDrawElements(GL_TRIANGLES, indexDataNextIndex, GL_UNSIGNED_SHORT, nullptr);


		globalDirtyFlag = false;

		return true; // UI was redrawn
	}

	return false; // UI was not redrawn
}

void free_ui_resources()
{
	bind_shader_program(0);
	glDeleteProgram(shaderProgram);
	glDeleteTextures(1, &asciiAtlas.textureId);

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vboId);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &iboId);

	glDeleteVertexArrays(1, &vaoId);

	delete font;
}

void get_window_dimensions(unsigned int & width, unsigned int & height)
{
	width = windowWidth;
	height = windowHeight;
}

}
