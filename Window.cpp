#include <Window.h>

#ifdef __linux__
#define GLFW_EXPOSE_NATIVE_X11
#else
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#ifdef __linux__
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#endif

#include <stdexcept>
#include <iostream>
#include <cstring>
#include <vector>
#include <cassert>

#ifdef _WIN32
#define EASYTAB_IMPLEMENTATION
#include "easytab.h"
#endif

using namespace std;


static GLFWwindow * window;

#ifdef __linux__
struct TabletInfo {
	// int tabletXMin = -1;
	// int tabletXMax = -1;
	// int tabletYMin = -1;
	// int tabletYMax = -1;
	int tabletPressureMin = -1;
	int tabletPressureMax = -1;
	int tabletTiltMin = -1;
	int tabletTiltMax = -1;

	int motionType;
	int stylusDownType;
	int stylusUpType;
};

static Display * xdisplay = nullptr;
static Window xwindow; // Only valid if xdisplay != nullptr
static vector<XDevice *> xdevices;

static TabletInfo tablet;

#else
static HWND win32Window;
#endif


static bool tabletDetected = false;
bool tablet_detected() { return tabletDetected; }

static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
	(void) source;
	(void) id;
	(void) severity;
	(void) length;
	(void) userParam;
	if(type != GL_DEBUG_TYPE_OTHER_ARB) {
		clog << message << endl;
	}
	assert(type != GL_DEBUG_TYPE_ERROR_ARB);
}

void create_window(unsigned int width, unsigned int height, unsigned int glVer)
{
	if (!glfwInit())
        throw runtime_error("Error initialising GLFW library");

	if(!glVer || glVer < 31) {
		glVer = 31;
	}

	if(glVer > 33 && glVer < 40) {
		glVer = 33;
	}

	if(glVer > 46) {
		glVer = 46;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glVer/10);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glVer%10);

	if(glVer >= 32) {
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // For OpenGL 3.2+ only
	}

	glfwWindowHint(GLFW_DEPTH_BITS, 0);
	glfwWindowHint(GLFW_STENCIL_BITS, 0);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    window = glfwCreateWindow(width, height, "Painting Application", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        throw runtime_error("Error creating window");
    }
    glfwSwapInterval(0);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_CULL_FACE);

	// All colours are stored in the linear colour space, OpenGL will convert the colours to srgb when drawing the canvas on the window
	glEnable(GL_FRAMEBUFFER_SRGB);

	if (GLAD_GL_ARB_debug_output) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glDebugMessageCallbackARB(debug_callback, nullptr);

		clog << "GL_ARB_debug_output is available" << endl;
	}
	if(GLAD_GL_ARB_clear_texture) {
		clog << "GL_ARB_clear_texture is available" << endl;
	}

	if(GLAD_GL_ARB_get_program_binary) {
		clog << "GL_ARB_get_program_binary is available" << endl;
	}

	if(GLAD_GL_ARB_timer_query) {
		clog << "GL_ARB_timer_query is available" << endl;
	}

	if(GLAD_GL_ARB_texture_storage) {
		clog << "GL_ARB_texture_storage is available" << endl;
	}

	#ifdef __linux__
	xdisplay = glfwGetX11Display();
	assert(xdisplay);
	xwindow = glfwGetX11Window(window);
	assert(xwindow);
	#else
	win32Window = glfwGetWin32Window(window);
	#endif
    
}

static window_resize_callback resizeCallback;

static void run_window_resize_callback(GLFWwindow * window, int w, int h)
{
	(void) window;
	if(resizeCallback) {
		resizeCallback(w, h);
	}
}

void assign_window_resize_callback(window_resize_callback callback)
{
	resizeCallback = callback;
	glfwSetWindowSizeCallback(window, run_window_resize_callback);

	int w, h;
	glfwGetWindowSize(window, &w, &h);
	callback(w, h);
}

//

static mouse_click_callback mouseClickCallback;

static void run_mouse_click_callback(GLFWwindow * window, int button, int action, int mods)
{
	(void) window;
	(void) mods;
	if(mouseClickCallback && button >= 0 && button <= 2 && (action == GLFW_RELEASE || action == GLFW_PRESS)) {
		double x, y;
		glfwGetCursorPos(window, &x, &y);

		mouseClickCallback(button, (unsigned int) x, (unsigned int) y, action == GLFW_PRESS);
	}
}

void assign_mouse_click_callback(mouse_click_callback callback)
{
	mouseClickCallback = callback;
	glfwSetMouseButtonCallback(window, run_mouse_click_callback);
}

//

static stylus_motion_callback stylusMotionCallback = nullptr;

void assign_stylus_motion_callback(stylus_motion_callback callback)
{
	stylusMotionCallback = callback;
}

//

static mouse_motion_callback mouseMotionCallback = nullptr;

static void run_mouse_motion_callback(GLFWwindow * window, double x, double y)
{
	(void)window;
	if(mouseMotionCallback) {
		mouseMotionCallback((unsigned int) x, (unsigned int) y);
	}
}

void assign_mouse_motion_callback(mouse_motion_callback callback)
{
	mouseMotionCallback = callback;
	glfwSetCursorPosCallback(window, run_mouse_motion_callback);	
}

//

static scroll_callback scrollCallback = nullptr;

static void run_scroll_callback(GLFWwindow * window, double x, double y)
{
	(void)window;
	if(scrollCallback) {
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		scrollCallback(mouseX, mouseY, (int) x, (int) y);
	}
}

void assign_scroll_callback(scroll_callback callback)
{
	scrollCallback = callback;
	glfwSetScrollCallback(window, run_scroll_callback);	
}

//

bool window_should_close()
{
	return glfwWindowShouldClose(window);
}

void swap_buffers()
{
	glfwSwapBuffers(window);
}

#ifdef __linux__

static Bool is_tablet_event (Display *display, XEvent *event, XPointer arg0)
{
	(void)display;
	(void)arg0;

	if(tablet.motionType == event->type) {
		return true;
	}

	return false;
}

void scan_tablet_events()
{
	if(tabletDetected && stylusMotionCallback) {
		XEvent event;

    	while(XCheckIfEvent(xdisplay, &event, is_tablet_event, nullptr)) {
    		XDeviceMotionEvent * motionEvent = (XDeviceMotionEvent *) &event;
    		float p;

    		if(tablet.tabletPressureMin) {
    			unsigned int range = tablet.tabletPressureMax - tablet.tabletPressureMin;
    			p = (motionEvent->axis_data[2] - (float)tablet.tabletPressureMin) / range;
    		}
    		else {
    			p = motionEvent->axis_data[2] / (float)tablet.tabletPressureMax;
    		}

    		if(p < 0.001f) {
    			p = 0;
    		}
    		else if (p >= 1.0f) {
    			p = 1.0f;
    		}
    		stylusMotionCallback(motionEvent->x, motionEvent->y, p);
    	}
	}
}
#else
static void scan_tablet_events()
{
	if(tabletDetected && stylusMotionCallback) {
		// This will not examine all tablet messages if other messages are mixed between them
		// This seems to be the beest that can be done without modifying GLFW

		MSG msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			if (EasyTab_HandleEvent(msg.hwnd, msg.message, msg.lParam, msg.wParam) == EASYTAB_OK) {
				// Remove tablet event from queue
				PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE);

				stylusMotionCallback(EasyTab->PosX, EasyTab->PosY, EasyTab->Pressure);
			}
			else {
				break;
			}
		}
	}
}
#endif

void wait_for_input(double timeout)
{
	scan_tablet_events();

	if(timeout == 0) {
		glfwWaitEvents();
	}
	else {
		glfwWaitEventsTimeout(timeout);
	}

	scan_tablet_events();
}

/* Tablet detection code */
// Adapted from https://github.com/ApoorvaJ/EasyTab/blob/master/easytab.h

#ifdef __linux__
void find_tablets()
{
	#define _assert(x) {if(!(x)){throw runtime_error("Error finding and initialising graphics tablet (X11,XI)");}}

	tabletDetected = false;


	// Close all devices, if a device is still connected it will be reopened.
	for(XDevice * device : xdevices) {
    	XCloseDevice(xdisplay, device);
	}
	xdevices.clear();	
	

    int ndevices;
    XDeviceInfo * info = XListInputDevices(xdisplay, &ndevices);
    _assert(info);
    _assert(ndevices > 0);


    // All tablet (stylus) devices are initialised so that they report motion events 

    for(unsigned int i = 0; i < (unsigned int) ndevices; i++) {
    	unsigned int nameLength = strlen(info[i].name);

    	if(nameLength < 6 || memcmp(&info[i].name[nameLength-6], "stylus", 6) != 0) continue;

    	clog << "Stylus detected: " << info[i].name << endl;

    	XDevice * device = XOpenDevice(xdisplay, info[i].id);
    	
    	if(!device) {
    		continue;
    	}

	    XAnyClassPtr classPtr = &info[i].inputclassinfo[0];

	    if(info[i].num_classes < 1) {
	    	XCloseDevice(xdisplay, device);
	    	continue;
	    }

    	xdevices.push_back(device);

    	for(unsigned int j = 0; j < (unsigned int) info[i].num_classes; j++) {
			XID c_class = classPtr->c_class;

			if(c_class == ValuatorClass) {

				XValuatorInfo * valuatorInfo = (XValuatorInfo *) classPtr;

				if(valuatorInfo->num_axes < 2) {
                    continue;
                }

				// tablet.tabletXMin = valuatorInfo->axes[0].min_value;
				// tablet.tabletXMax = valuatorInfo->axes[0].max_value;
				// tablet.tabletYMin = valuatorInfo->axes[1].min_value;
				// tablet.tabletYMax = valuatorInfo->axes[1].max_value;

				if(valuatorInfo->num_axes >= 3) {
					tablet.tabletPressureMin = valuatorInfo->axes[2].min_value;
					tablet.tabletPressureMax = valuatorInfo->axes[2].max_value;

					if(valuatorInfo->num_axes >= 4) {
						// This has not been tested and is based off of information in the linuxwacom github wiki
						tablet.tabletTiltMin = valuatorInfo->axes[3].min_value;
						tablet.tabletTiltMax = valuatorInfo->axes[3].max_value;
					}
				}

    			XEventClass eventClasses[3];
				DeviceMotionNotify(device, tablet.motionType, eventClasses[0]);
				// DeviceButtonPress(device, tablet.stylusDownType, eventClasses[1]);
				// DeviceButtonRelease(device, tablet.stylusUpType, eventClasses[2]);

				XSelectExtensionEvent(xdisplay, xwindow, eventClasses, 1 /*3*/);

				tabletDetected =  true;
				break;
			}

			classPtr = (XAnyClassPtr) (((uintptr_t)classPtr) + classPtr->length);
		}

		if(tabletDetected) {
			break;
		}
    }

    XFreeDeviceList(info);

    _assert(xdevices.size());

	#undef _assert
}
#else
void find_tablets()
{
	if(tabletDetected) {
		EasyTab_Unload();
	}

	tabletDetected = EasyTab_Load(win32Window) == EASYTAB_OK;
	if(!tabletDetected) {
		throw runtime_error("No tablet detected (Wintab API)");
	}
}
#endif

void set_cursor_pointer();
void set_cursor_ibeam();
void set_cursor_custom(void * data);

void close_window()
{
	#ifdef __linux__
	if(xdisplay) {
		for(XDevice * device : xdevices) {
	    	XCloseDevice(xdisplay, device);
		}
		xdevices.clear();
	}
	#else
	if(tabletDetected) {
		EasyTab_Unload();
	}
	#endif

	glfwTerminate();
}
