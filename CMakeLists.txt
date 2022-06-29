cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 17)
if (APPLE)
  enable_language(OBJC)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

project(AMC)

find_package(OpenGL REQUIRED)

add_subdirectory(libs/SDL)
add_subdirectory(libs/SDL_image)
add_subdirectory(libs/glew-cmake)
add_subdirectory(libs/glm)

list(APPEND CPP_FILES
  src/gl_helper.cpp src/hello_cube.cpp src/shader.cpp src/camera.cpp 
  src/standard.cpp src/world.cpp src/chunk.cpp src/player.cpp src/inventory.cpp 
  src/gui.cpp src/gui_element_visitor.cpp src/tool.cpp src/imgui.cpp src/imgui_widgets.cpp 
  src/imgui_draw.cpp src/imgui_demo.cpp src/imgui_tables.cpp src/imgui_impl_sdl.cpp
  src/imgui_impl_opengl3.cpp src/mesh.cpp src/rigidbody.cpp
)


add_executable(game src/main.cpp ${CPP_FILES})
target_link_libraries(game SDL2 libglew_static SDL2_image glm::glm)
target_include_directories(game PRIVATE ${OPENGL_INCLUDE_DIRS})