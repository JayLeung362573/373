cmake_minimum_required(VERSION 3.28.2)

add_library(Network
        WebsocketNetworking.cpp
        MessageTranslator.cpp)

target_include_directories(Network PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(Network PUBLIC
        networking
        Lobby)

target_compile_features(Network PUBLIC cxx_std_23)