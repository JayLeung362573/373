cmake_minimum_required(VERSION 3.28.2)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/tests")


FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
  WebSocketNetworkingTest.cpp
  GameServerTest.cpp
  Phase1IntegrationTest.cpp
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
  TypesTest.cpp
  RulesTest.cpp
  GameSpecLoaderTest.cpp
  LobbyRegistryTest.cpp
)

# Link with gtest (the Quickstart uses GTest::gtest_main)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main core_lib parser)

target_compile_features(unit_tests PRIVATE cxx_std_23)

target_include_directories(unit_tests PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/lib)
        
# Use CTest integration (can use to discover tests automatically)
include(GoogleTest)
gtest_discover_tests(unit_tests)
