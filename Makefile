CXX=g++
CC=gcc
SOURCES=Files.cpp Main.cpp png.cpp Shader.cpp Timer.cpp Window.cpp canvas/ArrayTexture.cpp canvas/Brush.cpp canvas/Canvas.cpp \
canvas/Framebuffer.cpp canvas/ImageBlock.cpp ui/Font.cpp ui/UI.cpp ui/Widgets.cpp ui/Button.cpp ui/Menu.cpp ui/Label.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LIBRARIES=glfw3 freetype2 libpng x11 xi

all: $(OBJECTS) lib/glad.o
	$(CXX) *.o lib/*.o ui/*.o canvas/*.o -o paint -O0 -g -fstack-protector-all `pkg-config --libs $(LIBRARIES)` -ldl

# Add '-DNDEBUG' to disable debug assertions
.cpp.o:
	$(CXX) -c -std=c++11 -Wall -Wextra -O0 -g -Wpedantic -Iinclude -fstack-protector-all `pkg-config --cflags $(LIBRARIES)` $< -o $@

lib/glad.o:
	$(CC) -c lib/glad.c -Iinclude -o lib/glad.o

clean:
	-rm *.o
	-rm ui/*.o
	-rm canvas/*.o


