#pragma once

// Creates a GLFW window, finds a graphics tablet, and assigns input handlers (see Input.cpp) for the input devices
void create_window(unsigned int width, unsigned int height, unsigned int glVersion = 0, unsigned int forceBitDepth = 0);

void set_window_icon(unsigned char * data, unsigned int width, unsigned int height);

void find_tablets();
bool window_should_close();
void swap_buffers();

// Pass 0 to wait without a timeout
void wait_for_input(double timeout = 0);

void set_cursor_pointer();
void set_cursor_ibeam();
void set_cursor_custom(void * data);
void close_window();

bool tablet_detected();

/* Callbacks */

// Width, Height
typedef void (* window_resize_callback) (unsigned int, unsigned int);
void assign_window_resize_callback(window_resize_callback);

// Button, x, y
typedef void (* mouse_click_callback) (unsigned int, unsigned int, unsigned int, bool);
void assign_mouse_click_callback(mouse_click_callback callback);

// x, y, pressure
typedef void (* stylus_motion_callback) (unsigned int, unsigned int, float);
void assign_stylus_motion_callback(stylus_motion_callback callback);

// x, y
typedef void (* mouse_motion_callback) (unsigned int, unsigned int);
void assign_mouse_motion_callback(mouse_motion_callback callback);

// mouse x, mouse y, x, y
typedef void (* scroll_callback) (unsigned int, unsigned int, int, int);
void assign_scroll_callback(scroll_callback callback);

void get_opengl_version(unsigned int & major, unsigned int & minor);

void get_window_dimensions(unsigned int & width, unsigned int & height);
