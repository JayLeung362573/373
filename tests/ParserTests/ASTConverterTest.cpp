#include "gtest/gtest.h"
#include "parser/ASTConverter.h"
#include "parser/NodeTypes.h"
#include "parser/TreeSitterUtil.h"
#include "parser/GameSpecLoader.h"
#include "GameSpecLoaderTest.h"

extern "C" const TSLanguage *tree_sitter_socialgaming();

TEST_F(GameSpecLoaderTest, ASTConverter_BasicAssignments) {
    std::string src = R"(
configuration {
  name: "Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  x <- "a";
  x <- 1;
}
    )";

    TSParser *parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_socialgaming());
    NodeType::init(tree_sitter_socialgaming());

    TSTree *tree = ts_parser_parse_string(parser, nullptr, src.c_str(), (uint32_t)src.size());
    TSNode root = ts_tree_root_node(tree);

    // Find rules node
    TSNode rules = TSNode{};
    uint32_t count = ts_node_child_count(root);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(root, i);
        if (ts_node_symbol(child) == NodeType::RULES) {
            rules = child;
            break;
        }
    }
    ASSERT_FALSE(ts_node_is_null(rules)) << "No rules block found";

// Get the body node inside rules
    ASSERT_GT(ts_node_named_child_count(rules), 0) << "Rules node has no children";
    TSNode body = ts_node_named_child(rules, 0);
    ASSERT_EQ(ts_node_symbol(body), NodeType::BODY) << "First child of rules should be body";

    // testing the conversion
    int converted = 0;
    uint32_t n = ts_node_named_child_count(body);
    for (uint32_t i = 0; i < n; ++i) {
        TSNode st = ts_node_named_child(body, i);

        if (ts_node_symbol(st) == NodeType::RULE)
            st = ts_node_named_child(st, 0);

        if (ts_node_symbol(st) == NodeType::ASSIGNMENT) {
            auto ast = ASTConverter::convertAssignment(src, st);
            EXPECT_TRUE(ast != nullptr);
            ++converted;
        }
    }

    EXPECT_EQ(converted, 2);

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}