cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 17)

project(AMC)

find_package(OpenGL REQUIRED)

add_subdirectory(libs/SDL)
add_subdirectory(libs/glew-cmake)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_definitions("$<$<CONFIG:DEBUG>: -glldb -Wall -Werror -fsanitze=address -Wndeprecated-declaration")
endif()

if (APPLE)
    add_compile_options("$<$<CONFIG:DEBUG>:-march=native>")
endif()

#add_compile_definitions("$<$<CONFIG:DEBUG>:-DDEBUG>")
#TODO: If compiler is msvc Copy SDL .dlls to exe dir or force sdl to statically link

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(demo main.cpp ${SRC_FILES})
target_link_libraries(demo SDL2 libglew_static)
target_include_directories(demo PRIVATE ${OPENGL_INCLUDE_DIRS})