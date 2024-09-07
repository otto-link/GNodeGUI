cmake_minimum_required(VERSION 3.10)
project(gnodegui-root VERSION 0.0.0)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC compiler options
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Od")
else()
  # GNU compiler options
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wno-dangling-reference -O3 -pthread"
  )
endif()

set(CMAKE_CXX_STANDARD 20)

# --- Dear ImGui
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# add_subdirectory(external)
add_subdirectory(GNodeGUI)
add_subdirectory(tests)
