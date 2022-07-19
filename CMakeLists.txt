cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 17)
if (APPLE)
  enable_language(OBJC)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

project(AMC)

find_package(OpenGL REQUIRED)

add_subdirectory(libs/SDL)
add_subdirectory(libs/SDL_image)
add_subdirectory(libs/glew-cmake)
add_subdirectory(libs/glm)
add_subdirectory(libs/googletest)

list(APPEND CPP_FILES
  src/gl_helper.cpp src/hello_cube.cpp src/shader.cpp src/camera.cpp 
  src/standard.cpp src/world.cpp src/chunk.cpp src/player.cpp src/inventory.cpp 
  src/gui.cpp src/gui_element_visitor.cpp src/tool.cpp src/imgui.cpp src/imgui_widgets.cpp 
  src/imgui_draw.cpp src/imgui_demo.cpp src/imgui_tables.cpp src/imgui_impl_sdl.cpp
  src/imgui_impl_opengl3.cpp src/mesh.cpp
  src/physics.cpp src/entity.cpp src/model.cpp
)


add_executable(game src/main.cpp ${CPP_FILES})
target_link_libraries(game SDL2 libglew_static SDL2_image glm::glm gtest)
target_include_directories(game PRIVATE ${OPENGL_INCLUDE_DIRS})

add_executable(animation_editor src/animation_editor.cpp ${CPP_FILES})
target_link_libraries(animation_editor SDL2 libglew_static SDL2_image glm::glm gtest)
target_include_directories(animation_editor PRIVATE ${OPENGL_INCLUDE_DIRS})

add_executable(test tests/test.cpp ${CPP_FILES})
target_link_libraries(test SDL2 libglew_static SDL2_image glm::glm gtest)
target_include_directories(test PRIVATE ${OPENGL_INCLUDE_DIRS})
