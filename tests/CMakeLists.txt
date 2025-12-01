cmake_minimum_required(VERSION 3.28.2)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests")

# Copy game files to build/tests/games for testing
file(COPY ${CMAKE_SOURCE_DIR}/games
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# Also copy to build/ so tests work when run from there
file(COPY ${CMAKE_SOURCE_DIR}/games
        DESTINATION ${PROJECT_BINARY_DIR})

FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
  WebSocketNetworkingTest.cpp
  GameServerTest.cpp
  GameInterpreterTests/AssignmentTest.cpp
  GameInterpreterTests/ComparisonTest.cpp
  GameInterpreterTests/ConstantTest.cpp
  GameInterpreterTests/DiscardTest.cpp
  GameInterpreterTests/ExtendTest.cpp
  GameInterpreterTests/InputTextStmtTest.cpp
  GameInterpreterTests/LogicalOperationTest.cpp
  GameInterpreterTests/ShuffleTest.cpp
  GameInterpreterTests/UnaryOperationTest.cpp
  GameInterpreterTests/SortTest.cpp
  GameInterpreterTests/MatchTest.cpp
  GameInterpreterTests/ProgramTest.cpp
  GameInterpreterTests/ForLoopTest.cpp
  GameInterpreterTests/CallableTest.cpp
  TypesTest.cpp
  RulesTest.cpp
  LobbyRegistryTest.cpp
  InputManagerTest.cpp
  GameSessionTest.cpp
  GameInterpreterSmokeTest.cpp
  ParserTests/GameSpecLoaderTest.cpp
  ParserTests/ASTConverterTest.cpp
  ParserTests/IntegrationTest.cpp
  ParserTests/RPSMinimalTest.cpp
  ParserTests/FullRPSParseTest.cpp
  ParserTests/RPSFullConversionTest.cpp
  ParserTests/InputDebugTest.cpp
  ParserTests/ForLoopDebugTest.cpp
  ParserTests/MessageScoresDebugTest.cpp
  ParserTests/MethodCallDebugTest.cpp
  ParserTests/GameSpecLoaderTest.h
)

# Link with gtest (the Quickstart uses GTest::gtest_main)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main core_lib parser)

target_compile_features(unit_tests PRIVATE cxx_std_23)


target_include_directories(unit_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/lib)

target_compile_definitions(unit_tests PRIVATE
        GAMES_DIR=\"${PROJECT_SOURCE_DIR}/games\")

# Use CTest integration (can use to discover tests automatically)
include(GoogleTest)

gtest_discover_tests(unit_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)



gtest_discover_tests(unit_tests)

add_executable(Logger_test
  Logger_test.cpp
)
target_link_libraries(Logger_test PRIVATE GTest::gtest_main core_lib spdlog::spdlog)
target_include_directories(Logger_test PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/lib)
target_compile_features(Logger_test PRIVATE cxx_std_23)
include(GoogleTest)
gtest_discover_tests(Logger_test)
