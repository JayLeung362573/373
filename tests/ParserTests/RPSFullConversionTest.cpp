#include <gtest/gtest.h>
#include "parser/GameSpecLoader.h"
#include <filesystem>

TEST(RPSFullConversionTest, LoadRockPaperScissors) {
    std::filesystem::path filePath = std::filesystem::path(GAMES_DIR) / "rock-paper-scissors.game";
    GameSpecLoader loader;
    GameSpec spec = loader.loadFile(filePath.string().c_str());

    EXPECT_EQ(spec.name, "Rock, Paper, Scissors");
    EXPECT_FALSE(spec.constants.empty());
    EXPECT_FALSE(spec.variables.empty());

    EXPECT_EQ(spec.rulesProgram.size(), 0);
}

TEST(RPSFullConversionTest, LoadGameWithSupportedFeatures) {
    std::filesystem::path filePath = std::filesystem::path(GAMES_DIR) / "test-supported.game";
    GameSpecLoader loader;
    GameSpec spec = loader.loadFile(filePath.string().c_str());

    EXPECT_EQ(spec.name, "Test Game");
    EXPECT_FALSE(spec.constants.empty());
    EXPECT_FALSE(spec.variables.empty());

    EXPECT_EQ(spec.rulesProgram.size(), 12);
}
