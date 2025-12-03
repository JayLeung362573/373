cmake_minimum_required(VERSION 3.28.2)

add_library(GameServer
        GameServer.cpp
        GameServer.h)

target_include_directories(GameServer PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        )

target_compile_features(GameServer PUBLIC cxx_std_23)

target_link_libraries(GameServer PUBLIC
        GameEngine
        GameSession
        Lobby
        Network
        parser)