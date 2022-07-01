# CMakeList.txt : CMake project for 3DRoguelike, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (3DRoguelike "3DRoguelike.cpp" "3DRoguelike.h")

target_link_libraries(3DRoguelike PRIVATE spdlog::spdlog spdlog::spdlog_header_only)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 3DRoguelike PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
