cmake_minimum_required(VERSION 3.16)
project(Engine2D CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(engine2d
    src/main.cpp
    src/renderer.cpp
    src/shader.cpp
)

target_include_directories(engine2d PRIVATE ${SDL2_INCLUDE_DIRS} src)
target_link_libraries(engine2d PRIVATE ${SDL2_LIBRARIES} GLEW::GLEW OpenGL::GL)
