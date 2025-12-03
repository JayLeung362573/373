// #include <gtest/gtest.h>
// #include "parser/ASTConverter.h"
// #include "parser/NodeTypes.h"
// #include "parser/TreeSitterUtil.h"
// #include "Rules.h"
// #include "GameEngine/GameInterpreter.h"
// extern "C" const TSLanguage *tree_sitter_socialgaming();
//
// TEST(ASTConverterSmokeTest, BasicAssignment) {
//     std::string src = R"(
//         rules { x <- 1 }
//     )";
//
//     TSParser *parser = ts_parser_new();
//     ts_parser_set_language(parser, tree_sitter_socialgaming());
//     NodeType::init(tree_sitter_socialgaming());
//     TSTree *tree = ts_parser_parse_string(parser, nullptr, src.c_str(), (uint32_t)src.size());
//     TSNode root = ts_tree_root_node(tree);
//
//     TSNode rules = ts_node_named_child(root, 0);
//     TSNode stmt = ts_node_named_child(rules, 0);
//     if (ts_node_symbol(stmt) == NodeType::RULE)
//         stmt = ts_node_named_child(stmt, 0);
//
//     TSNode lhs = ts_node_named_child(stmt, 0);
//     TSNode rhs = ts_node_named_child(stmt, 1);
//
//     auto lhsExpr = ASTConverter::convertExpression(src, lhs);
//     auto rhsExpr = ASTConverter::convertExpression(src, rhs);
//     auto asg = ast::makeAssignment(std::move(lhsExpr), std::move(rhsExpr));
//
//     GameInterpreter interp;
//     EXPECT_NO_THROW(interp.visit(*asg));
//
//     ts_tree_delete(tree);
//     ts_parser_delete(parser);
// }