cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 17)

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/cpp-pm/hunter/archive/v0.23.272.tar.gz"
    SHA1 "02cdafca02692376d55e549acf85a34445907a43"
)

set(CMAKE_CXX_COMPILER /usr/local/opt/llvm@11/bin/clang++)

project(Hope)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -glldb -Wall -Wno-unused-const-variable -Werror -fsanitize=address")

hunter_add_package(msgpack)
find_package(msgpack CONFIG REQUIRED)
list(APPEND TEMP_LIBRARIES msgpack::msgpack)

hunter_add_package(SDL2)
find_package(SDL2 CONFIG REQUIRED)
list(APPEND TEMP_LIBRARIES SDL2::SDL2)

hunter_add_package(SDL_image)
find_package(SDL_image CONFIG REQUIRED)
list(APPEND TEMP_LIBRARIES SDL_image::SDL_image)

hunter_add_package(SDL_ttf)
find_package(SDL_ttf CONFIG REQUIRED)
list(APPEND TEMP_LIBRARIES SDL_ttf::SDL_ttf)

hunter_add_package(vorbis)
find_package(vorbis CONFIG REQUIRED)
list(APPEND TEMP_LIBRARIES vorbis::vorbis)

hunter_add_package(SDL_mixer)
find_package(SDL_mixer CONFIG REQUIRED vorbis::vorbis)
list(APPEND TEMP_LIBRARIES SDL_mixer::SDL_mixer)

hunter_add_package(glew)
find_package(glew CONFIG REQUIRED)
list(APPEND TEMP_LIBRARIES glew::glew)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})

hunter_add_package(glm)
find_package(glm REQUIRED)
list(APPEND TEMP_LIBRARIES glm)

add_executable(game src/main.cpp src/gl_helper.cpp src/hello_cube.cpp src/shader.cpp src/camera.cpp src/gui.cpp src/standard.cpp src/world.cpp src/chunk.cpp)
target_link_libraries(game PUBLIC ${TEMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

string(TIMESTAMP CURRENT_DATE_TIME "%Y_%m_%d_%H_%M_%S")

add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_LIST_DIR}/saves/
    ${CMAKE_CURRENT_LIST_DIR}/backup/${CURRENT_DATE_TIME}/
)