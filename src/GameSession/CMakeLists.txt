cmake_minimum_required(VERSION 3.28.2)

add_library(GameSession
        GameSession.cpp
        GameSession.h)

target_include_directories(GameSession PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        )

target_compile_features(GameSession PUBLIC cxx_std_23)

target_link_libraries(GameSession PUBLIC GameEngine)