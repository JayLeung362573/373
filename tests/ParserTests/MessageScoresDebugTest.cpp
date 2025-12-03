#include <gtest/gtest.h>
#include <tree_sitter/api.h>
#include <iostream>

extern "C" const TSLanguage *tree_sitter_socialgaming();

static void printNode(const std::string& src, TSNode node, int depth = 0) {
    std::string indent(depth * 2, ' ');
    const char* type = ts_node_type(node);
    TSSymbol symbol = ts_node_symbol(node);

    std::cout << indent << type << " (symbol=" << symbol << ")";

    if (ts_node_named_child_count(node) == 0) {
        uint32_t start = ts_node_start_byte(node);
        uint32_t end = ts_node_end_byte(node);
        std::string text = src.substr(start, end - start);
        if (text.size() > 50) text = text.substr(0, 47) + "...";
        std::cout << " = \"" << text << "\"";
    }
    std::cout << std::endl;

    uint32_t childCount = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_named_child(node, i);
        const char* fieldName = ts_node_field_name_for_child(node, i);
        if (fieldName) {
            std::cout << indent << "  [field: " << fieldName << "]" << std::endl;
        }
        printNode(src, child, depth + 1);
    }
}

TEST(MessageScoresDebugTest, ExploreMessageStructure) {
    std::string gameSpec = R"(
configuration {
  name: "Message Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  message all "Game starting!";
  message all "Round {round}";
}
)";

    TSParser *parser = ts_parser_new();
    const TSLanguage* language = tree_sitter_socialgaming();
    ts_parser_set_language(parser, language);

    TSTree *tree = ts_parser_parse_string(parser, nullptr, gameSpec.c_str(), gameSpec.size());
    ASSERT_NE(tree, nullptr);

    TSNode root = ts_tree_root_node(tree);
    std::cout << "\n=== MESSAGE CST STRUCTURE ===" << std::endl;
    printNode(gameSpec, root);

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}

TEST(MessageScoresDebugTest, ExploreScoresStructure) {
    std::string gameSpec = R"(
configuration {
  name: "Scores Test"
  player range: (1, 1)
  audience: false
  setup: {}
}
constants {}
variables {}
per-player {}
per-audience {}
rules {
  scores ["wins"];
  scores ["wins", "losses"];
}
)";

    TSParser *parser = ts_parser_new();
    const TSLanguage* language = tree_sitter_socialgaming();
    ts_parser_set_language(parser, language);

    TSTree *tree = ts_parser_parse_string(parser, nullptr, gameSpec.c_str(), gameSpec.size());
    ASSERT_NE(tree, nullptr);

    TSNode root = ts_tree_root_node(tree);
    std::cout << "\n=== SCORES CST STRUCTURE ===" << std::endl;
    printNode(gameSpec, root);

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}
