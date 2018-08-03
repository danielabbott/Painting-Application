#include <hwcaps.h>
#include <glad/glad.h>

unsigned int max_texture_size()
{
	static int max = 1000000;
	if(max == 1000000) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);

		if(max < 1024) {
			max = 1024;
		}
		else if (max > 32768) {
			max = 32768;
		}
	}
	return max;
}
