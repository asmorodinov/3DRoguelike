# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("3DRoguelike")

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")

find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(CGAL CONFIG REQUIRED)
find_package(freetype CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# Include sub-projects.
add_subdirectory ("3DRoguelike")
