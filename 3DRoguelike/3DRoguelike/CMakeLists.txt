# CMakeList.txt : CMake project for 3DRoguelike, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (3DRoguelike "3DRoguelike.cpp" "3DRoguelike.h" "Game/Camera.h" "Game/Shader.h" "Game/Camera.cpp" "Game/Shader.cpp" "Game/Assets.h" "Game/Assets.cpp" "Game/Texture.h" "Game/Texture.cpp" "Game/Assert.h" "Game/Renderer.h" "Game/Renderer.cpp" "Game/Utility/Random.h" "Game/Utility/Random.cpp" "Game/Dungeon/Dungeon.h" "Game/Dungeon/Dungeon.cpp" "Game/Dungeon/Tile.h" "Game/Dungeon/Tile.cpp" "Game/Utility/Vector3D.h" "Game/Utility/Vector3D.cpp" "Game/Dungeon/WorldGrid.h" "Game/Dungeon/WorldGrid.cpp" "Game/Dungeon/TileRenderer.h" "Game/Dungeon/TileRenderer.cpp" "Game/Utility/GLError.h" "Game/Utility/GLError.cpp" "Game/Dungeon/Room.h" "Game/Dungeon/Room.cpp" "Game/Algorithms/Pathfind.h" "Game/Algorithms/Pathfind.cpp" "Game/Algorithms/Delaunay3D.h" "Game/Algorithms/Delaunay3D.cpp" "Game/Algorithms/MST.h" "Game/Algorithms/MST.cpp" "Game/UI/RenderText.h" "Game/UI/RenderText.cpp" "Game/Physics/CollisionDetection.h" "Game/Physics/CollisionDetection.cpp"   "Game/Utility/LogDuration.h" "Game/Physics/Entity.h" "Game/Physics/Entity.cpp" "Game/Physics/PlayerCollision.h" "Game/Physics/PlayerCollision.cpp" "Game/Model/Model.h" "Game/Model/Model.cpp" "Game/Model/OBJModel.h" "Game/Model/OBJModel.cpp" "Game/Model/ModelConverter.h" "Game/Model/ModelConverter.cpp" "Game/Utility/PathToResources.h" "Game/Algorithms/PersistentHashSet.h" "Game/Utility/HAMT.h" "Game/Utility/HAMT.cpp" "External/PersistentSet/External/memory-allocators/src/PoolAllocator.cpp"  "Game/Utility/CompareFiles.h" "Game/Utility/CompareFiles.cpp")

target_include_directories(3DRoguelike PRIVATE ${STB_INCLUDE_DIRS})
target_include_directories(3DRoguelike PRIVATE "External/SPARTA/include")
target_include_directories(3DRoguelike PRIVATE "External/ikos/core/include")
target_include_directories(3DRoguelike PRIVATE "External/patricia/include")
target_include_directories(3DRoguelike PRIVATE "External/PersistentSet/PersistentSet")
target_include_directories(3DRoguelike PRIVATE "External/PersistentSet/Allocators")

target_link_libraries(3DRoguelike PRIVATE glm::glm)
target_link_libraries(3DRoguelike PRIVATE glfw)
target_link_libraries(3DRoguelike PRIVATE glad::glad)
target_link_libraries(3DRoguelike PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
target_link_libraries(3DRoguelike PRIVATE CGAL::CGAL)
target_link_libraries(3DRoguelike PRIVATE freetype)
target_link_libraries(3DRoguelike PRIVATE yaml-cpp)
target_link_libraries(3DRoguelike PRIVATE immer)
target_link_libraries(3DRoguelike PRIVATE Boost::boost)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET 3DRoguelike PROPERTY CXX_STANDARD 20)
endif()

if (MSVC)
  set_target_properties(3DRoguelike PROPERTIES LINK_FLAGS "/PROFILE")
  set_target_properties(3DRoguelike PROPERTIES LINK_FLAGS "/DEBUG:FULL")

  # target_compile_options(3DRoguelike PRIVATE /Wall)
endif()

# TODO: Add tests and install targets if needed.
