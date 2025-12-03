#include <gtest/gtest.h>
#include "parser/GameSpecLoader.h"
#include "parser/ASTConverter.h"
#include "parser/NodeTypes.h"
#include "src/GameEngine/GameInterpreter.h"
#include "src/GameEngine/InputManager.h"

extern "C" const TSLanguage *tree_sitter_socialgaming();

TEST(RPSTest, DISABLED_MinimalRPSFeatures) {
    std::string gameSpec = R"(
configuration {
  name: "Minimal RPS Test"
  player range: (2, 4)
  audience: false
  setup: {}
}
constants {
  weapons: [
    { name: "Rock", beats: "Scissors" }
  ]
}
variables {
  winners: []
}
per-player {
  wins: 0
}
per-audience {}
rules {
  discard 1 from winners;
  extend winners with players;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);

    EXPECT_EQ(spec.name, "Minimal RPS Test");
    EXPECT_GT(spec.rulesProgram.size(), 0) << "Should have parsed some rules";

    Program program;
    for (auto& stmt : spec.rulesProgram) {
        program.statements.push_back(std::move(stmt));
    }

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, std::move(program));
    EXPECT_NO_THROW(interpreter.execute());
}

TEST(RPSTest, SimpleDiscardStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Discard"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  discard 1 from winners;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);

    ASSERT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 discard statement";

    // verify it's actually a Discard with correct parameters
    auto* discard = static_cast<ast::Discard*>(spec.rulesProgram[0].get());

    // check target is Variable "winners"
    auto* targetVar = static_cast<ast::Variable*>(discard->getTarget());
    EXPECT_EQ(targetVar->getName().name, "winners");

    // check amount is Constant 1
    auto* amountConst = static_cast<ast::Constant*>(discard->getAmount());
    EXPECT_TRUE(amountConst->getValue().isInteger());
    EXPECT_EQ(amountConst->getValue().asInteger().value, 1);
}

TEST(RPSTest, SimpleExtendStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Extend"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  extend winners with players;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);

    std::cout << "Parsed " << spec.rulesProgram.size() << " statements" << std::endl;
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 extend statement";
}

TEST(RPSTest, SimpleReverseStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Reverse"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  reverse deck;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 reverse statement";
}

TEST(RPSTest, SimpleShuffleStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Shuffle"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  shuffle deck;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 shuffle statement";
}

TEST(RPSTest, SimpleSortStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Sort"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  sort scores;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 sort statement";
}

TEST(RPSTest, SimpleMatchStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Match"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  match true {
    true => {
      x <- 1;
    }
    false => {
      y <- 2;
    }
  }
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 match statement";
}

TEST(RPSTest, ComparisonOperators) {
    std::string gameSpec = R"(
configuration {
  name: "Test Comparison"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  match x = 5 {
    true => { a <- 1; }
  }
  match y < 10 {
    true => { b <- 2; }
  }
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 2) << "Should have 2 match statements with comparisons";
}

TEST(RPSTest, LogicalAndUnaryOperators) {
    std::string gameSpec = R"(
configuration {
  name: "Test Logical"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  match p || q {
    true => { a <- 1; }
  }
  match !done {
    true => { b <- 2; }
  }
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 2) << "Should have 2 match statements with logical/unary ops";
}

TEST(RPSTest, InputTextStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Input Text"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  input text to player {
    prompt: "Enter your name"
    target: player.name
  }
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 input text statement";
}

TEST(RPSTest, InputChoiceStatement) {
    std::string gameSpec = R"(
configuration {
  name: "Test Input Choice"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  input choice to player {
    prompt: "Choose your weapon"
    choices: weapons
    target: player.weapon
  }
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 1) << "Should have 1 input choice statement";
}

TEST(RPSTest, ForLoop) {
    std::string gameSpec = R"(
configuration {
  name: "Test ForLoop"
  player range: (1, 4)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  for player in players {
    x <- 1;
  }
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);

    EXPECT_EQ(spec.rulesProgram.size(), 1);
}

TEST(RPSTest, UnsupportedMessageSkipped) {
    std::string gameSpec = R"(
configuration {
  name: "Test Message"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  message all "Hello!";
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 0) << "Message should be skipped (unsupported)";
}

TEST(RPSTest, UnsupportedMethodCallSkipped) {
    std::string gameSpec = R"(
configuration {
  name: "Test Method Call"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  x <- players.size();
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);
    EXPECT_EQ(spec.rulesProgram.size(), 0) << "Method call should be skipped (unsupported)";
}
