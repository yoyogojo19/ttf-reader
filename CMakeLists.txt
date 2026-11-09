cmake_minimum_required(VERSION 3.10)
project(ttfReader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(ttfReader scr/main.cpp)

target_link_libraries(ttfReader
        PRIVATE
        glfw
        GLEW::GLEW
        OpenGL::GL
)
target_include_directories(ttfReader
        PRIVATE
        ${GLEW_INCLUDE_DIRS}
        ${GLFW3_INCLUDE_DIRS}
)