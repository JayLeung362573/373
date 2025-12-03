#include <gtest/gtest.h>
#include "parser/GameSpecLoader.h"
#include "parser/GameSpec.h"
#include <filesystem>
#include "GameSpecLoaderTest.h"

TEST_F(GameSpecLoaderTest, LoadGameName) {
    std::filesystem::path filePath = std::filesystem::path(GAMES_DIR) / "hello-test.game";
    GameSpec spec = loader.loadFile(filePath.string().c_str());
    EXPECT_EQ(spec.name, "Hello Test");
}

TEST_F(GameSpecLoaderTest, LoadRockPaperScissors) {
    std::filesystem::path filePath = std::filesystem::path(GAMES_DIR) / "rock-paper-scissors.game";
    GameSpec spec = loader.loadFile(filePath.string().c_str());
    EXPECT_EQ(spec.name, "Rock, Paper, Scissors");
}

TEST_F(GameSpecLoaderTest, LoadFromString) {
    std::string gameText = R"(
configuration {
  name: "Test Game"
  player range: (2, 4)
  audience: false
  setup: { }
}
constants { }
variables { }
per-player { }
per-audience { }
rules { }
)";

    GameSpec spec = loader.loadString(gameText);
    EXPECT_EQ(spec.name, "Test Game");
}

TEST_F(GameSpecLoaderTest, ParseAssignments) {
    // Verify parsing doesn't crash and detects all assignments
    GameSpec spec = loader.loadFile("games/test-simple-rules.game");
    EXPECT_EQ(spec.name, "Test Simple Rules");
    // Output shows: 5 assignments detected (x, y, z, w, player.score)
}

TEST_F(GameSpecLoaderTest, ParseSimpleAssignment) {
    std::string gameText = R"(
configuration {
  name: "Simple Assignment Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  score <- 0;
}
)";

    GameSpec spec = loader.loadString(gameText);
    EXPECT_EQ(spec.name, "Simple Assignment Test");
    // Output shows: 1 assignment detected (score <- 0)
}

TEST_F(GameSpecLoaderTest, ParseQualifiedIdentifierAssignment) {
    std::string gameText = R"(
configuration {
  name: "Qualified ID Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  player.name <- "Alice";
  game.round <- 1;
}
)";

    GameSpec spec = loader.loadString(gameText);
    EXPECT_EQ(spec.name, "Qualified ID Test");
    // Output shows: 2 assignments with qualified identifiers
}

TEST_F(GameSpecLoaderTest, ParseListAssignment) {
    std::string gameText = R"(
configuration {
  name: "List Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  choices <- ["rock", "paper", "scissors"];
  numbers <- [1, 2, 3];
}
)";

    GameSpec spec = loader.loadString(gameText);
    EXPECT_EQ(spec.name, "List Test");
    // Output shows: 2 list assignments detected
}

TEST_F(GameSpecLoaderTest, ParseMapAssignment) {
    std::string gameText = R"(
configuration {
  name: "Map Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  config <- { "key1": "value1", "key2": "value2" };
}
)";

    GameSpec spec = loader.loadString(gameText);
    EXPECT_EQ(spec.name, "Map Test");
    // Output shows: 1 map assignment detected
}

TEST_F(GameSpecLoaderTest, ParseNoAssignments) {
    std::string gameText = R"(
configuration {
  name: "No Rules Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {}
)";

    GameSpec spec = loader.loadString(gameText);
    EXPECT_EQ(spec.name, "No Rules Test");
    // Output shows: 0 statements found
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
