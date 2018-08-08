// A crude prototype UI is implemented here to test various functionality within the application

#include <Window.h>
#include <UI.h>
#include <stdexcept>
#include <iostream>
#include <glad/glad.h>
#include <chrono>
#include <thread>
#include <ImageBlock.h>
#include <Layer.h>
#include <Canvas.h>
#include <Timer.h>
#include <iomanip>

using namespace std;

void testfunc_clear_layer2();

class MyButton : public UI::Button
{
	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		UI::Button::onMouseButtonReleased(button);
		if(!button) {
			testfunc_clear_layer2();
		}
		return true; 
	}
public:
	MyButton(string text_, LeftRightAlignment leftRightTextAlign = LEFT_RIGHT_CENTRE, TopBottomAlignment topBottomTextAlign = TOP_BOTTOM_CENTRE)
	: UI::Button(text_, leftRightTextAlign, topBottomTextAlign) {}
};

class LayerButton : public UI::Label
{
	unsigned int layerIndex;

	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		UI::Label::onMouseButtonReleased(button);
		if(!button) {
			set_active_layer(layerIndex);
		}
		return true; 
	}

	virtual uint32_t getBackGroundColour() override
	{
		if(layerIndex == get_active_layer()) {
			return 0x30300000;
		}
		else {
			return 0;
		}
	}
public:
	LayerButton(string text_, unsigned int layerIndex_) : UI::Label(text_), layerIndex(layerIndex_) {}
};

int main(int argc, char ** argv)
{

	unsigned int glVer = 0;
	unsigned int forceBitDepth = 0;

	for(int i = 1; i < argc; i++) {
		if(		argv[i][0] == '-' 
			&& (argv[i][1] == 'G' || argv[i][1] == 'g')
			&& (argv[i][2] == 'L' || argv[i][2] == 'l'))
		{
			char n1 = argv[i][3];
			if(!n1 || n1 < '0' || n1 > '9') break;

			char n2 = argv[i][4];
			if(!n2 || n2 < '0' || n2 > '9') n2 = '0';

			if((n1 == '3' && n2 >= '1') || n1 == '4') {
				glVer = (n1-'0')*10 + n2-'0';
				clog << "Using GL version: " << glVer << endl;
			}
			else {
				clog << "GL version must be >= 3.1" << endl;
			}
		}
		else if (argv[i][0] == '-' 
			&& (argv[i][1] == 'B' || argv[i][1] == 'b'))
		{
			forceBitDepth = (unsigned int) strtoul (&argv[i][2], nullptr, 10);

			if(forceBitDepth) {
				clog << "Using bit depth: " << forceBitDepth << endl;
			}
		}
	}

	create_window(640, 480, glVer, forceBitDepth);
	create_opengl_timer();

	try {
    	find_tablets();
	} catch(const runtime_error & e) {
		clog << "No tablets detected" << endl;
	}

	create_layers();


	UI::initialise_ui();




	MyButton button1 = MyButton("press me");
	MyButton button2 = MyButton("or me");
	MyButton button3 = MyButton("or maybe this very long button right here");
	MyButton button4 = MyButton("exclamation marks are broken > ! <");
	UI::MenuBar container = UI::MenuBar(vector<UI::Widget *> { &button1, &button2, &button3, &button4 }, 1, 0, 0, 0, 0xff202020, UI::Container::FLOW_ACROSS);

	// UI::Label lbl = UI::Label("hiya");
	MyButton b1 = MyButton("1");
	MyButton b2 = MyButton("2");
	MyButton b3 = MyButton("333333333");
	UI::Menu menu = UI::Menu(vector<UI::Widget *> { &b1, &b2, &b3 });

	UI::MenuItem button12345 = UI::MenuItem("12345", &menu);
	UI::MenuBar container2 = UI::MenuBar(vector<UI::Widget *> { &button12345 }, 1, 2, 0, 0, 0xff404040, UI::Container::FLOW_ACROSS);

	Canvas canvas = Canvas(1, 1, 0, 0);

	vector<UI::Widget *> layerLabels;

	for(unsigned int i = 0; i < 64; i++) {
		Layer & layer = get_layer(i);

		if(layer.type == Layer::LAYER) {
			layerLabels.insert(layerLabels.begin(), new LayerButton(layer.name, i));
		}

	}

	layerLabels.insert(layerLabels.begin(), new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::NONE));
	layerLabels.push_back(new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::NONE));
	UI::Container layersContainer = UI::Container(layerLabels, 0, 1, 100, 0, 0xff202020, UI::Container::FLOW_DOWN);



	UI::Container root = UI::Container(vector<UI::Widget *> { &container,&canvas,&container2,&layersContainer }, 0, 0, 640, 480, 0, UI::Container::BORDER);
	set_root_container(&root);


	unsigned int x, y, canvasWidth, canvasHeight;
	canvas.getArea(x, y, canvasWidth, canvasHeight);
	initialise_canvas_display(canvasWidth/2, canvasHeight/2);

	auto lastUpdateTime = chrono::high_resolution_clock::now();

	while(!window_should_close()) {
		start_opengl_timer();
		bool redrawn = UI::draw_ui(false);

		uint64_t time = stop_opengl_timer();

		if(redrawn) {
			swap_buffers();
			glFinish();

			if(time) {
				clog << "Time to update: " << setprecision(3) << (time / 1000000.0f) << " ms" << endl;
			}
		}

		// Limit frame rate to 30 fps when user is moving the mouse/stylus
		auto now = chrono::high_resolution_clock::now();
		unsigned int elapsedTime = chrono::duration_cast<chrono::microseconds>(now - lastUpdateTime).count();
		if(elapsedTime < 33333) {
			this_thread::sleep_for(chrono::microseconds(33333-elapsedTime));
			lastUpdateTime = chrono::high_resolution_clock::now();
		}
		else {
			lastUpdateTime = now;
		}

		wait_for_input();


	}

	free_canvas_resources();
	Brush::freeStaticResources();
	UI::free_ui_resources();
	close_window();
	return 0;
}
