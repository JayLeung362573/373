cmake_minimum_required(VERSION 3.28.2)

project(main)

# Default to Debug build if not specified
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()

# Enable sanitizers only for Debug builds, but retain static casts enforcement
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Running Debug build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti -fsanitize=address,undefined") # Enable Address sanitizer and enforce static casts
else()
    message(STATUS "Running Production build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")

# For Remote dependencies
include(cmake/CPM.cmake)

CPMAddPackage(
  NAME web-socket-networking
  GITHUB_REPOSITORY nsumner/web-socket-networking
  GIT_TAG master
  OPTIONS
    "ENABLE_FLUTTER_CLIENT OFF"
    "ENABLE_FTXUI_CLIENT OFF"
    "ENABLE_NCURSES_CLIENT ON"
)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.14.1
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Tree-sitter C runtime library - using FetchContent
include(FetchContent)

FetchContent_Declare(
  tree-sitter
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
  GIT_TAG        v0.26.0-pre-61-gb3bc7701
)

FetchContent_MakeAvailable(tree-sitter)

# Build tree-sitter library from fetched content
add_library(treesitter STATIC
  ${tree-sitter_SOURCE_DIR}/lib/src/lib.c
)
target_include_directories(treesitter PUBLIC
  ${tree-sitter_SOURCE_DIR}/lib/include
)

# Add parser subdirectory which contains the grammar and loader
add_subdirectory(parser)

# Add src and tests subdirectories
if(EXISTS "${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")
  add_subdirectory(src)
endif()

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()