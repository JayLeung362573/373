cmake_minimum_required(VERSION 3.28.2)
add_library(GameEngine
  GameInterpreter.cpp
  Rules.cpp
)

target_include_directories(GameEngine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(GameEngine PUBLIC cxx_std_23)
target_compile_options(GameEngine PRIVATE -frtti) # for dynamic casts