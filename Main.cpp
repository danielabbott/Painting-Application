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
#include <MainWindow.h>

using namespace std;

Canvas * canvas;

extern string webpLibraryFile;

void parse_args(int argc, char ** argv, unsigned int & glVer, unsigned int & forceBitDepth)
{
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
}

int main(int argc, char ** argv)
{
	unsigned int glVer = 0;
	unsigned int forceBitDepth = 0;

	parse_args(argc, argv, glVer, forceBitDepth);

	create_window(1024, 768, glVer, forceBitDepth);
	create_opengl_timer();

	try {
    	find_tablets();
	} catch(const runtime_error & e) {
		clog << "No tablets detected" << endl;
	}


	UI::initialise_ui();
	canvas = new Canvas(1, 1, 1024, 768); // TODO: GUI layout could change, separate canvas widget from actual canvas data


	MainWindow mainWindow(canvas);
	set_root_container(mainWindow.getRoot());


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
