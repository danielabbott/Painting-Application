#include <glad/glad.h>
#include <cassert>

static GLuint queryId = 0;
static bool timing = false;

void create_opengl_timer()
{
	#ifndef NDEBUG
	assert(!queryId);
	glGenQueries(1, &queryId);
	#endif
}

void start_opengl_timer()
{
	#ifndef NDEBUG
	if (GLAD_GL_ARB_timer_query) { // Core in OpenGL 3.3
		assert(queryId);
		assert(!timing);

		glBeginQuery(GL_TIME_ELAPSED, queryId);
		timing = true;
	}
	#endif
}

uint64_t stop_opengl_timer()
{
	#ifndef NDEBUG
	if (GLAD_GL_ARB_timer_query) {
		assert(queryId);
		assert(timing);

		glEndQuery(GL_TIME_ELAPSED);
		timing = false;

		uint64_t value;
		glGetQueryObjectui64v(queryId, GL_QUERY_RESULT, &value);
		return value;
	}
	#endif

	return 0;
}
