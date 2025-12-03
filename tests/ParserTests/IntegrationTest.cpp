#include <gtest/gtest.h>
#include "parser/GameSpecLoader.h"
#include "parser/ASTConverter.h"
#include "parser/NodeTypes.h"
#include "src/GameEngine/GameInterpreter.h"
#include "src/GameEngine/InputManager.h"

extern "C" const TSLanguage *tree_sitter_socialgaming();

TEST(IntegrationTest, ManualPipelineTest) {
    std::string gameSpec = R"(
configuration {
  name: "Integration Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  x <- 5;
  y <- 10;
  z <- 15;
}
)";

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_socialgaming());
    NodeType::init(tree_sitter_socialgaming());

    TSTree *tree = ts_parser_parse_string(parser, nullptr, gameSpec.c_str(), (uint32_t)gameSpec.size());
    TSNode root = ts_tree_root_node(tree);

    TSNode rules = TSNode{};
    uint32_t count = ts_node_child_count(root);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(root, i);
        if (ts_node_symbol(child) == NodeType::RULES) {
            rules = child;
            break;
        }
    }
    ASSERT_FALSE(ts_node_is_null(rules));

    ASSERT_GT(ts_node_named_child_count(rules), 0);
    TSNode body = ts_node_named_child(rules, 0);
    ASSERT_EQ(ts_node_symbol(body), NodeType::BODY);

    Program program;
    uint32_t statementCount = ts_node_named_child_count(body);

    for (uint32_t i = 0; i < statementCount; ++i) {
        TSNode stmt = ts_node_named_child(body, i);

        if (ts_node_symbol(stmt) == NodeType::RULE) {
            stmt = ts_node_named_child(stmt, 0);
        }

        if (ts_node_symbol(stmt) == NodeType::ASSIGNMENT) {
            auto astNode = ASTConverter::convertAssignment(gameSpec, stmt);
            program.statements.push_back(std::move(astNode));
        }
    }

    ASSERT_EQ(program.statements.size(), 3);

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, std::move(program));
    EXPECT_NO_THROW(interpreter.execute());

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}

TEST(IntegrationTest, FullPipelineWithGameSpecLoader) {
    std::string gameSpec = R"(
configuration {
  name: "Full Pipeline Test"
  player range: (2, 4)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  round <- 1;
  score <- 100;
}
)";

    GameSpecLoader loader;
    GameSpec spec = loader.loadString(gameSpec);

    EXPECT_EQ(spec.name, "Full Pipeline Test");
    EXPECT_EQ(spec.rulesProgram.size(), 2);

    Program program;
    for (auto& stmt : spec.rulesProgram) {
        program.statements.push_back(std::move(stmt));
    }

    InputManager inputManager;
    GameInterpreter interpreter(inputManager, std::move(program));
    EXPECT_NO_THROW(interpreter.execute());
}
