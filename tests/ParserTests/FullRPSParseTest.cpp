#include <gtest/gtest.h>
#include <tree_sitter/api.h>
#include <fstream>
#include <sstream>
#include <filesystem>

extern "C" const TSLanguage *tree_sitter_socialgaming();

TEST(FullRPSParseTest, TreeSitterParsesRPSWithoutErrors) {
    // Load the actual rock-paper-scissors.game file
    std::filesystem::path filePath = std::filesystem::path(GAMES_DIR) / "rock-paper-scissors.game";
    std::ifstream file(filePath);
    ASSERT_TRUE(file.is_open()) << "Could not open " << filePath;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string gameSpec = buffer.str();

    TSParser *parser = ts_parser_new();
    const TSLanguage* language = tree_sitter_socialgaming();
    ts_parser_set_language(parser, language);

    TSTree *tree = ts_parser_parse_string(parser, nullptr, gameSpec.c_str(), gameSpec.size());
    ASSERT_NE(tree, nullptr) << "Tree-sitter failed to parse RPS";

    TSNode root = ts_tree_root_node(tree);

    // Check if there are any parsing errors
    EXPECT_FALSE(ts_node_has_error(root)) << "Tree-sitter found syntax errors in RPS";

    // Verify we got a valid game node
    const char* nodeType = ts_node_type(root);
    EXPECT_STREQ(nodeType, "game") << "Root node should be 'game', got: " << nodeType;

    std::cout << "Successfully parsed rock-paper-scissors.game with tree-sitter" << std::endl;
    std::cout << "Root node type: " << nodeType << std::endl;

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}
