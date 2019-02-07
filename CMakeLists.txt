cmake_minimum_required(VERSION 3.1)

project(paintapp)


# Remove for release
#add_compile_definitions(_DEBUG)

set (CMAKE_CXX_STANDARD 11)

include_directories(include)

if(MINGW)
    include_directories(windows_include)
endif(MINGW)


if(MINGW)
    add_link_options(-L.. -lglfw3 -lpng -lz -lfreetype)
endif(MINGW)

add_executable(paintapp Files.cpp Main.cpp png.cpp webp.cpp Shader.cpp Timer.cpp Window.cpp hwcaps.cpp canvas/ArrayTexture.cpp canvas/Brush.cpp canvas/Canvas.cpp canvas/Framebuffer.cpp canvas/ImageBlock.cpp canvas/CanvasSetup.cpp canvas/CanvasDraw.cpp ui/Font.cpp ui/UI.cpp ui/Widgets.cpp ui/Button.cpp ui/Menu.cpp ui/Label.cpp ui/MainWindow.cpp lib/glad.c)


target_compile_options(paintapp PRIVATE -Wall -Wextra -pedantic -O0 -DGLFW_DLL -DYEEEE)

set_source_files_properties(lib/glad.c PROPERTIES COMPILE_FLAGS -w)


if(NOT MINGW)
    target_link_libraries(paintapp glfw3 freetype2 libpng x11 xi)
endif(NOT MINGW)

install(TARGETS paintapp RUNTIME DESTINATION bin)
