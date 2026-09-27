cmake_minimum_required(VERSION 3.16)
project(LOCO VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(LOCO_BUILD_EDITOR "Build the LOCO editor and project manager" ON)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
if(LOCO_BUILD_EDITOR)
    add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
    target_include_directories(imgui PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES} OpenGL::GL)
endif()

add_library(loco_runtime STATIC
    src/core/project.cpp
    src/platform/input.cpp
    src/drivers/gl/shader.cpp
    src/drivers/gl/framebuffer.cpp
    src/drivers/gl/gl_rendering_server.cpp
    src/main/engine.cpp
)

target_include_directories(loco_runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(loco_runtime PUBLIC
    ${SDL2_LIBRARIES}
    GLEW::GLEW
    OpenGL::GL
)

if(LOCO_BUILD_EDITOR)
    target_compile_definitions(loco_runtime PUBLIC LOCO_EDITOR_ENABLED)
    target_link_libraries(loco_runtime PUBLIC imgui)
endif()

if(LOCO_BUILD_EDITOR)
    add_library(loco_editor STATIC
        src/editor/editor.cpp
        src/editor/project_manager.cpp
    )
    target_link_libraries(loco_editor PUBLIC loco_runtime imgui)
endif()

add_executable(loco src/main/main.cpp)
if(LOCO_BUILD_EDITOR)
    target_link_libraries(loco PRIVATE loco_editor)
else()
    target_link_libraries(loco PRIVATE loco_runtime)
endif()

file(GLOB_RECURSE LOCO_ALL_SOURCES src/*.h src/*.cpp)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${LOCO_ALL_SOURCES})

message(STATUS "LOCO ${PROJECT_VERSION} - editor: ${LOCO_BUILD_EDITOR}")
