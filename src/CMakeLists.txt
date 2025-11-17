cmake_minimum_required(VERSION 3.28.2)

add_subdirectory(GameEngine)
add_subdirectory(Lobby)
add_subdirectory(GameSession)

add_library(core_lib
  GameClient.cpp
  GameServer.cpp
  WebsocketNetworking.cpp
  MessageTranslator.cpp
)

target_include_directories(core_lib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(core_lib PUBLIC
        GameEngine
        networking
        Lobby
        GameSession)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE core_lib)
target_compile_features(main PRIVATE cxx_std_23)
