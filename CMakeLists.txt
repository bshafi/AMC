cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER /usr/local/opt/llvm@12/bin/clang++)
set(CMAKE_C_COMPILER /usr/local/opt/llvm@12/bin/clang)

include("cmake/HunterGate.cmake")

if (NOT HUNTER_URL AND NOT HUNTER_SHA1)
  file(
    DOWNLOAD https://api.github.com/repos/cpp-pm/hunter/releases/latest
    ${CMAKE_BINARY_DIR}/LatestGate.json)

  file(READ ${CMAKE_BINARY_DIR}/LatestGate.json latestGateJson)
  string(REGEX MATCH "tag_name\": \"([a-z0-9.]+)\"" tagName "${latestGateJson}")

  set(hunterUrlTarball "https://github.com/cpp-pm/hunter/archive/${CMAKE_MATCH_1}.tar.gz")
  set(hunterLocalTarballName "${CMAKE_BINARY_DIR}/hunter-${CMAKE_MATCH_1}.tar.gz")

  file(DOWNLOAD ${hunterUrlTarball} "${hunterLocalTarballName}")

  file(SHA1 "${hunterLocalTarballName}" HUNTER_SHA1)
  set(HUNTER_URL "file://${hunterLocalTarballName}")
endif()

HunterGate(
    URL ${HUNTER_URL}
    SHA1 ${HUNTER_SHA1}
)

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

hunter_add_package(OpenAL)
find_package(OpenAL CONFIG REQUIRED)
list(APPEND OpenAL::OpenAL)

list(APPEND CPP_FILES
  src/gl_helper.cpp src/hello_cube.cpp src/shader.cpp src/camera.cpp 
  src/standard.cpp src/world.cpp src/chunk.cpp src/player.cpp src/inventory.cpp 
  src/gui.cpp src/gui_element_visitor.cpp src/tool.cpp src/imgui.cpp src/imgui_widgets.cpp 
  src/imgui_draw.cpp src/imgui_demo.cpp src/imgui_tables.cpp src/imgui_impl_sdl.cpp
  src/imgui_impl_opengl3.cpp
)

add_executable(game src/main.cpp ${CPP_FILES})
target_link_libraries(game PUBLIC ${TEMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

add_executable(test src/test.cpp ${CPP_FILES})
target_link_libraries(test PUBLIC ${TEMP_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})

string(TIMESTAMP CURRENT_DATE_TIME "%Y_%m_%d_%H_%M_%S")

add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_LIST_DIR}/saves/
    ${CMAKE_CURRENT_LIST_DIR}/backup/${CURRENT_DATE_TIME}/
)