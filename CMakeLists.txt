cmake_minimum_required(VERSION 3.16)
project(LOCO CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

add_executable(engine2d
    src/main.cpp
    src/engine.cpp
    src/gl_rendering_server.cpp
    src/framebuffer.cpp
    src/shader.cpp
    src/input.cpp
    src/editor.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(engine2d PRIVATE
    ${SDL2_INCLUDE_DIRS}
    src
    ${IMGUI_DIR}
)

target_link_libraries(engine2d PRIVATE ${SDL2_LIBRARIES} GLEW::GLEW OpenGL::GL)