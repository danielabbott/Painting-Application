#pragma once

#include <cstdint>

void create_opengl_timer();

// Does nothing if the OpenGL context does not support timer queries
void start_opengl_timer();

void stop_opengl_timer();

// Returns 0 if the OpenGL context does not support timer queries
// Time is in nanoseconds
uint64_t get_opengl_timer_value();
