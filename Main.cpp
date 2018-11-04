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
#include <cstring>

using namespace std;

Canvas * canvas;

class MyButton : public UI::Button
{
	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		UI::Button::onMouseButtonReleased(button);
		if(!button) {
			// clear_layer(get_first_layer()->next);
			canvas->fill_layer(canvas->get_first_layer(), 0xffffffff);
		}
		return true; 
	}
public:
	MyButton(string text_)
	: UI::Button(text_, LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment::TOP_BOTTOM_CENTRE) {}
};

class InputToggleButton : public UI::Button
{
	bool useMouse = true;

	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		UI::Button::onMouseButtonReleased(button);
		if(!button) {
			useMouse = !useMouse;
			if(!tablet_detected()) {
				useMouse = true;
			}
			set_canvas_input_device(useMouse);
		}
		return true; 
	}

	virtual string const& getText()
	{
		static string mouseString = "Mouse";
		static string tabletString = "Tablet";
		if(useMouse) {
			return mouseString;
		}
		else {
			return tabletString;
		}
	}
public:
	InputToggleButton()
	: UI::Button("Mouse", LeftRightAlignment::LEFT_RIGHT_CENTRE, TopBottomAlignment::TOP_BOTTOM_CENTRE) {
		set_canvas_input_device(true);
	}
};

class LayerButton : public UI::Label
{
	Layer * layer;

	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		UI::Label::onMouseButtonReleased(button);
		if(!button) {
			canvas->set_active_layer(layer);
		}
		return true; 
	}

	virtual uint32_t getBackGroundColour() override
	{
		if(layer == canvas->get_active_layer()) {
			return 0x30300000;
		}
		else {
			return 0;
		}
	}
public:
	LayerButton(Layer * layer_) : UI::Label(layer_->name, 0, 0, 150, 0), layer(layer_) {}
};

class LayerVisibilityButton : public UI::Label
{
	Layer * layer;

	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		UI::Label::onMouseButtonReleased(button);
		if(!button) {
			layer->visible = !layer->visible;
			canvas->forceRedraw();
		}
		return true; 
	}

	virtual string const& getText()
	{
		static string plusString = "+";
		static string hyphenString = "-";
		if(layer->visible) {
			return plusString;
		}
		else {
			return hyphenString;
		}
	}
public:
	LayerVisibilityButton(Layer * layer_) : UI::Label(layer_->name, 1, 0, 50, 0), layer(layer_) {}
};

class ColourSetter : public UI::Button
{
	float colour[3];

	virtual bool onMouseButtonReleased(unsigned int button) override
	{ 
		cout<<button<<endl;
		UI::Button::onMouseButtonReleased(button);
		if(!button) {
			set_active_colour(colour[0], colour[1], colour[2], 0.3f);
		}
		return true; 
	}

public:
	ColourSetter(string t_, float r, float g, float b) : UI::Button(t_) 
	{
		colour[0] = r;
		colour[1] = g;
		colour[2] = b;
	}
};

extern string webpLibraryFile;

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
		else if (argv[i][0] == '-' && argv[i][1] == '-'
			&& strncmp(&argv[i][2], "webp-library-file=", 18) == 0)
		{
			unsigned int l = 0;
			unsigned int j = 2+18;

			while(argv[i][j] && argv[i][j] != ' ') {
				j++;
				l++;
			}

			webpLibraryFile = string(&argv[i][2+18], l);

			clog << "Webp encoder/decoder implementation search path set to " << webpLibraryFile << endl;
		}
		else if (argv[i][0] == '-' 
			&& (argv[i][1] == 'f' || argv[i][1] == 'F'))
		{
			unsigned int fontSize = (unsigned int) strtoul(&argv[i][2], nullptr, 10);

			if(fontSize < 8) {
				clog << "Invalid font size. Using size 8" << endl;
				fontSize = 8;
			}
			else if(fontSize > 32) {
				clog << "Invalid font size. Using size 32" << endl;
				fontSize = 32;
			}
			else {
				clog << "Using font size " << fontSize << endl;
			}
			UI::set_font_size(fontSize);
		}

		else {
			cout<<&argv[i][2]<<endl;
		}
	}

	create_window(1024, 768, glVer, forceBitDepth);
	create_opengl_timer();

	try {
    	find_tablets();
	} catch(const runtime_error & e) {
		clog << "No tablets detected" << endl;
	}



	UI::initialise_ui();


	InputToggleButton inp;
	MyButton button1("Sample button");
	UI::MenuBar container(vector<UI::Widget *> { &inp, &button1 }, 1, 0, 0, 0, 0xff202020, UI::Container::LayoutManager::FLOW_ACCROSS);

	MyButton b1("1");
	MyButton b2("2");
	MyButton b3("333333333");
	UI::Menu menu(vector<UI::Widget *> { &b1, &b2, &b3 });

	UI::MenuItem button12345("12345", &menu);
	ColourSetter red("Red", 1, 0, 0);
	ColourSetter green("Green", 0, 1, 0);
	ColourSetter blue("Blue", 0, 0, 1);
	UI::MenuBar container2(vector<UI::Widget *> { &button12345,&red,&green,&blue }, 1, 2, 0, 0, 0xff404040, UI::Container::LayoutManager::FLOW_ACCROSS);

	canvas = new Canvas(1, 1, 0, 0);

	vector<UI::Widget *> layerLabels;

	Layer * layer = canvas->get_first_layer();
	while(1) {
		if(layer->type == Layer::Type::LAYER) {
			UI::Widget * layerNameButton = new LayerButton(layer);
			UI::Widget * layerVisibilityButton = new LayerVisibilityButton(layer);

			UI::Container * c = new UI::Container(vector<UI::Widget *>{layerNameButton, layerVisibilityButton},
				0, layerLabels.size(), 200, 0, 0, UI::Container::LayoutManager::FLOW_ACCROSS);

			layerLabels.insert(layerLabels.begin(), c); 
		}

		if(layer->next) {
			layer = layer->next;
		}
		else {
			break;
		}
	}

	layerLabels.insert(layerLabels.begin(), new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::LayoutManager::NONE));
	layerLabels.push_back(new UI::Container(vector<UI::Widget *> {}, 0, 0, 0, 5, 0xff000000, UI::Container::LayoutManager::NONE));
	UI::Container layersContainer(layerLabels, 0, 1, 0, 0, 0xff202020, UI::Container::LayoutManager::FLOW_DOWN);



	UI::Container root(vector<UI::Widget *> { &container,canvas,&container2,&layersContainer }, 0, 0, 0, 0, 0, UI::Container::LayoutManager::BORDER);
	set_root_container(&root);

	unsigned int x, y, canvasWidth, canvasHeight;
	canvas->getArea(x, y, canvasWidth, canvasHeight);
	canvas->initialise_canvas_display(canvasWidth/2, canvasHeight/2);

	auto lastUpdateTime = chrono::high_resolution_clock::now();

	while(!window_should_close()) {
		start_opengl_timer();
		bool redrawn = UI::draw_ui(false);
		stop_opengl_timer();


		if(redrawn) {
			swap_buffers();
			glFinish();

			uint64_t time = get_opengl_timer_value();
			if(time) {
				clog << "Time to update: " << setprecision(3) << (time / 1000000.0f) << " ms" << endl;
			}
		}

		// { // make the application quit after 2 frames
		// 	// Quit application after second frame
		// 	static int i = 0;
		// 	i++;
		// 	if(i == 3) break;
		// }

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

	canvas->freeCanvasResources();
	free_canvas_resources();
	Brush::freeStaticResources();
	UI::free_ui_resources();
	close_window();
	return 0;
}
