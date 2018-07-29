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
	virtual bool onMouseButtonReleased(unsigned int button) override { 
		UI::Button::onMouseButtonReleased(button);
		if(!button) {
			testfunc_clear_layer2();
		}
		return true; 
	}
public:
	MyButton(string text_) : UI::Button(text_) {}
};

int main(int argc, char ** argv)
{

	unsigned int glVer = 0;

	for(int i = 1; i < argc; i++) {
		if(		argv[i][0] == '-' 
			&& (argv[i][1] == 'G' || argv[i][1] == 'g')
			&& (argv[i][2] == 'L' || argv[i][2] == 'l'))
		{
			char n1 = argv[i][3];
			char n2 = argv[i][4];

			if((n1 >= '3') && (n2 >= '0' && n2 <= '9')) {
				glVer = (n1-'0')*10 + n2-'0';
			}
		}
	}

	create_window(640, 480, glVer);
	create_opengl_timer();

	try {
    	find_tablets();
	} catch(const runtime_error & e) {
		clog << "No tablets detected" << endl;
	}

	create_layers();


	UI::initialise_ui();

	// TODO use border layout


	MyButton button = MyButton("press me");
	UI::MenuBar container = UI::MenuBar(vector<UI::Widget *> { &button }, 0, 0, 640, 14, 0xff202020, UI::Container::NONE);

	UI::Label lbl = UI::Label("hiya");
	UI::Menu menu = UI::Menu(vector<UI::Widget *> { &lbl });

	UI::MenuItem button2 = UI::MenuItem("12345", &menu);
	UI::MenuBar container2 = UI::MenuBar(vector<UI::Widget *> { &button2 }, 0, 480-14, 640, 14, 0xff404040, UI::Container::NONE);

	Canvas canvas = Canvas(640, 480-14-14);
	UI::Container canvasContainer = UI::Container( vector<UI::Widget *> { &canvas }, 0, 14, 640, 480-14-14, 0, UI::Container::NONE);

	UI::Container root = UI::Container(vector<UI::Widget *> { &container,&canvasContainer,&container2 }, 0, 14, 640, 480-14-14, 0, UI::Container::NONE);
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

			if(time) {
				clog << "Time to update: " << setprecision(3) << (time / 1000000.0f) << " ms" << endl;
			}
		}

		wait_for_input();

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

	}

	free_canvas_resources();
	Brush::freeStaticResources();
	UI::free_ui_resources();
	close_window();
	return 0;
}
