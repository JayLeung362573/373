cmake_minimum_required(VERSION 3.28.2)

add_library(Lobby
        Lobby.cpp
        LobbyRegistry.cpp)

target_include_directories(Lobby PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(Lobby PUBLIC cxx_std_23)