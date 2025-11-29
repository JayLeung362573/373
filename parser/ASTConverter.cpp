#include "ASTConverter.h"
#include "NodeTypes.h"
#include "ParserUtil.h"
#include <cstring>
#include <unordered_map>

namespace {
    // dispatch table for statement conversions - lazy init to ensure NodeType symbols are ready
    using ConverterFunc = std::unique_ptr<ast::Statement>(*)(const std::string&, TSNode);

    const std::unordered_map<TSSymbol, ConverterFunc>& getStatementConverters() {
        static const std::unordered_map<TSSymbol, ConverterFunc> converters = {
            {NodeType::ASSIGNMENT, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertAssignment(s, n); }},
            {NodeType::DISCARD, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertDiscard(s, n); }},
            {NodeType::EXTEND, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertExtend(s, n); }},
            {NodeType::REVERSE, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertReverse(s, n); }},
            {NodeType::SHUFFLE, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertShuffle(s, n); }},
            {NodeType::SORT, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertSort(s, n); }},
            {NodeType::MATCH, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertMatch(s, n); }},
            {NodeType::INPUT_CHOICE, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertInputChoice(s, n); }},
            {NodeType::INPUT_TEXT, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertInputText(s, n); }},
            {NodeType::INPUT_RANGE, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertInputRange(s, n); }},
            {NodeType::INPUT_VOTE, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertInputVote(s, n); }},
            {NodeType::FOR, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertForLoop(s, n); }},
            {NodeType::PARALLEL_FOR, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertParallelFor(s, n); }},
            {NodeType::MESSAGE, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertMessage(s, n); }},
            {NodeType::SCORES, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertScores(s, n); }},
            {NodeType::COMMENT, [](const std::string& s, TSNode n) -> std::unique_ptr<ast::Statement> { return ASTConverter::convertComment(s, n); }},
        };
        return converters;
    }

    // extract string from prompt expression node (grammar wraps it but AST expects raw string)
    String extractPromptString(const std::string& src, TSNode promptNode) {
        auto promptExpr = ASTConverter::convertExpression(src, promptNode);
        auto* constantNode = static_cast<ast::Constant*>(promptExpr.get());
        if (!constantNode->getValue().isString()) {
            throw std::runtime_error("Input prompt must be a string literal");
        }
        return constantNode->getValue().asString();
    }
}

std::string ASTConverter::extractText(const std::string &src, TSNode node) {
    return slice(src, node);
}

std::string ASTConverter::extractQuotedString(const std::string &src, TSNode node) {
    return parseQuotedString(src, node);
}

std::unique_ptr<ast::Expression>
ASTConverter::convertExpression(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);
    const char* type = ts_node_type(node);

    // expression nodes can wrap operators or just wrap other expressions
    if (symbol == NodeType::EXPRESSION) {
        uint32_t namedCount = ts_node_named_child_count(node);
        uint32_t totalCount = ts_node_child_count(node);

        // check if this is an operator expression by looking at anonymous children
        if (totalCount > namedCount) {
            // has anonymous children - might be operators
            for (uint32_t i = 0; i < totalCount; ++i) {
                TSNode child = ts_node_child(node, i);
                if (!ts_node_is_named(child)) {
                    std::string op = slice(src, child);

                    // binary operators
                    if (op == "=" && namedCount == 2) {
                        auto left = convertExpression(src, ts_node_named_child(node, 0));
                        auto right = convertExpression(src, ts_node_named_child(node, 1));
                        return std::make_unique<ast::Comparison>(std::move(left), std::move(right), ast::Comparison::Kind::EQ);
                    }
                    if (op == "<" && namedCount == 2) {
                        auto left = convertExpression(src, ts_node_named_child(node, 0));
                        auto right = convertExpression(src, ts_node_named_child(node, 1));
                        return std::make_unique<ast::Comparison>(std::move(left), std::move(right), ast::Comparison::Kind::LT);
                    }
                    if (op == "+" && namedCount == 2) {
                        auto left = convertExpression(src, ts_node_named_child(node, 0));
                        auto right = convertExpression(src, ts_node_named_child(node, 1));
                        return std::make_unique<ast::ArithmeticOperation>(std::move(left), std::move(right), ast::ArithmeticOperation::Kind::ADD);
                    }
                    if (op == "||" && namedCount == 2) {
                        auto left = convertExpression(src, ts_node_named_child(node, 0));
                        auto right = convertExpression(src, ts_node_named_child(node, 1));
                        return std::make_unique<ast::LogicalOperation>(std::move(left), std::move(right), ast::LogicalOperation::Kind::OR);
                    }

                    // unary operators
                    if (op == "!" && namedCount == 1) {
                        auto operand = convertExpression(src, ts_node_named_child(node, 0));
                        return std::make_unique<ast::UnaryOperation>(std::move(operand), ast::UnaryOperation::Kind::NOT);
                    }
                }
            }
        }

        // check for method calls (builtin + argument_list)
        for (uint32_t i = 0; i < namedCount; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (ts_node_symbol(child) == NodeType::BUILTIN) {
                std::string methodName = slice(src, child);
                throw std::runtime_error("Method call ." + methodName + "() not supported by interpreter yet");
            }
        }

        // not an operator, just unwrap
        if (namedCount > 0) {
            return convertExpression(src, ts_node_named_child(node, 0));
        }
        throw std::runtime_error("Expression node has no children");
    }

    // constants - ints, strings, bools
    // note: grammar produces "number" nodes, even though we register INTEGER symbol
    if (symbol == NodeType::INTEGER || symbol == NodeType::QUOTED_STRING ||
        symbol == NodeType::BOOLEAN || strcmp(type, "number") == 0) {
        return convertConstant(src, node);
    }

    if (symbol == NodeType::IDENTIFIER) {
        return convertVariable(src, node);
    }

    if (symbol == NodeType::QUALIFIED_IDENTIFIER) {
        // tree-sitter sometimes wraps simple identifiers in qualified_identifier with just 1 child
        uint32_t childCount = ts_node_named_child_count(node);
        if (childCount == 1) {
            TSNode child = ts_node_named_child(node, 0);
            return convertExpression(src, child);
        }
        return convertQualifiedIdentifier(src, node);
    }

    if (symbol == NodeType::LIST_LITERAL || symbol == NodeType::VALUE_MAP) {
        return convertConstant(src, node);
    }

    throw std::runtime_error("Unknown expression type: " + std::string(ts_node_type(node)));
}

std::unique_ptr<ast::Constant>
ASTConverter::convertConstant(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);
    const char* type = ts_node_type(node);

    // grammar produces "number" nodes for integers
    if (symbol == NodeType::INTEGER || strcmp(type, "number") == 0) {
        int intValue = parseInteger(src, node);
        return std::make_unique<ast::Constant>(Value{Integer{intValue}});
    }

    if (symbol == NodeType::QUOTED_STRING) {
        std::string strValue = parseQuotedString(src, node);
        return std::make_unique<ast::Constant>(Value{String{strValue}});
    }

    if (symbol == NodeType::BOOLEAN) {
        bool boolValue = parseBoolean(src, node);
        return std::make_unique<ast::Constant>(Value{Boolean{boolValue}});
    }

    if (symbol == NodeType::LIST_LITERAL) {
        Value listValue = convertListLiteral(src, node);
        return std::make_unique<ast::Constant>(listValue);
    }

    if (symbol == NodeType::VALUE_MAP) {
        Value mapValue = convertValueMap(src, node);
        return std::make_unique<ast::Constant>(mapValue);
    }

    throw std::runtime_error("Unknown constant type: " + std::string(ts_node_type(node)));
}

std::unique_ptr<ast::Variable>
ASTConverter::convertVariable(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::IDENTIFIER) {
        throw std::runtime_error("Expected IDENTIFIER node for variable");
    }

    std::string varName = extractText(src, node);
    return std::make_unique<ast::Variable>(Name{varName});
}

std::unique_ptr<ast::Attribute>
ASTConverter::convertQualifiedIdentifier(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::QUALIFIED_IDENTIFIER) {
        throw std::runtime_error("Expected QUALIFIED_IDENTIFIER node");
    }

    // qualified identifier = things like "player.name" or "game.round"
    // has 2 children: base and attribute
    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 2) {
        throw std::runtime_error("QUALIFIED_IDENTIFIER should have exactly 2 named children");
    }

    TSNode baseNode = ts_node_named_child(node, 0);
    TSNode attrNode = ts_node_named_child(node, 1);

    // base could be nested like "game.player.score"
    std::unique_ptr<ast::Expression> baseExpr = convertExpression(src, baseNode);
    std::string attrName = extractText(src, attrNode);

    return std::make_unique<ast::Attribute>(std::move(baseExpr), String{attrName});
}

Value ASTConverter::convertValue(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);
    const char* type = ts_node_type(node);

    // grammar produces "number" nodes for integers
    if (symbol == NodeType::INTEGER || strcmp(type, "number") == 0) {
        int intValue = parseInteger(src, node);
        return Value{Integer{intValue}};
    }

    if (symbol == NodeType::QUOTED_STRING) {
        std::string strValue = parseQuotedString(src, node);
        return Value{String{strValue}};
    }

    if (symbol == NodeType::BOOLEAN) {
        bool boolValue = parseBoolean(src, node);
        return Value{Boolean{boolValue}};
    }

    if (symbol == NodeType::LIST_LITERAL) {
        return convertListLiteral(src, node);
    }

    if (symbol == NodeType::VALUE_MAP) {
        return convertValueMap(src, node);
    }

    throw std::runtime_error("Unknown value type: " + std::string(ts_node_type(node)));
}

Value ASTConverter::convertListLiteral(const std::string &src, TSNode node) {
    // [1, 2, 3] type stuff
    List<Value> list;

    uint32_t childCount = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_named_child(node, i);
        Value val = convertValue(src, child);
        list.value.push_back(val);
    }

    return Value{list};
}

Value ASTConverter::convertValueMap(const std::string &src, TSNode node) {
    // { "key": "value" } type stuff
    Map<String, Value> map;

    uint32_t childCount = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < childCount; i += 2) {
        if (i + 1 >= childCount) break;

        TSNode keyNode = ts_node_named_child(node, i);
        TSNode valueNode = ts_node_named_child(node, i + 1);

        std::string keyStr = parseQuotedString(src, keyNode);
        Value val = convertValue(src, valueNode);

        map.value[String{keyStr}] = val;
    }

    return Value{map};
}

std::unique_ptr<ast::Statement>
ASTConverter::convertStatement(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    // grammar wraps statements in "rule" nodes, unwrap them
    if (symbol == NodeType::RULE) {
        if (ts_node_named_child_count(node) > 0) {
            node = ts_node_named_child(node, 0);
            symbol = ts_node_symbol(node);
        }
    }

    const auto& converters = getStatementConverters();
    auto it = converters.find(symbol);
    if (it != converters.end()) {
        return it->second(src, node);
    }

    throw std::runtime_error("Unknown statement type: " + std::string(ts_node_type(node)));
}

std::unique_ptr<ast::Assignment>
ASTConverter::  convertAssignment(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::ASSIGNMENT) {
        throw std::runtime_error("Expected ASSIGNMENT node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 2) {
        throw std::runtime_error("ASSIGNMENT should have exactly 2 named children (target and value)");
    }

    TSNode targetNode = ts_node_named_child(node, 0);
    TSNode valueNode = ts_node_named_child(node, 1);

    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);
    std::unique_ptr<ast::Expression> valueExpr = convertExpression(src, valueNode);

    return std::make_unique<ast::Assignment>(std::move(targetExpr), std::move(valueExpr));
}

std::unique_ptr<ast::Discard>
ASTConverter::convertDiscard(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::DISCARD) {
        throw std::runtime_error("Expected DISCARD node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 2) {
        throw std::runtime_error("DISCARD should have 2 named children (amount and target)");
    }

    TSNode amountNode = ts_node_named_child(node, 0);
    TSNode targetNode = ts_node_named_child(node, 1);

    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);
    std::unique_ptr<ast::Expression> amountExpr = convertExpression(src, amountNode);

    return std::make_unique<ast::Discard>(std::move(targetExpr), std::move(amountExpr));
}

std::unique_ptr<ast::Extend>
ASTConverter::convertExtend(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::EXTEND) {
        throw std::runtime_error("Expected EXTEND node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 2) {
        throw std::runtime_error("EXTEND should have 2 named children (target and value)");
    }

    TSNode targetNode = ts_node_named_child(node, 0);
    TSNode valueNode = ts_node_named_child(node, 1);

    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);
    std::unique_ptr<ast::Expression> valueExpr = convertExpression(src, valueNode);

    return std::make_unique<ast::Extend>(std::move(targetExpr), std::move(valueExpr));
}

std::unique_ptr<ast::Reverse>
ASTConverter::convertReverse(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::REVERSE) {
        throw std::runtime_error("Expected REVERSE node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 1) {
        throw std::runtime_error("REVERSE should have 1 named child (target)");
    }

    TSNode targetNode = ts_node_named_child(node, 0);
    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);

    return std::make_unique<ast::Reverse>(std::move(targetExpr));
}

std::unique_ptr<ast::Shuffle>
ASTConverter::convertShuffle(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::SHUFFLE) {
        throw std::runtime_error("Expected SHUFFLE node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 1) {
        throw std::runtime_error("SHUFFLE should have 1 named child (target)");
    }

    TSNode targetNode = ts_node_named_child(node, 0);
    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);

    return std::make_unique<ast::Shuffle>(std::move(targetExpr));
}

std::unique_ptr<ast::Sort>
ASTConverter::convertSort(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::SORT) {
        throw std::runtime_error("Expected SORT node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount < 1 || childCount > 2) {
        throw std::runtime_error("SORT should have 1 or 2 named children");
    }

    TSNode targetNode = ts_node_named_child(node, 0);
    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);

    // optional sort key
    std::optional<String> key;
    if (childCount == 2) {
        TSNode keyNode = ts_node_named_child(node, 1);
        std::string keyStr = extractText(src, keyNode);
        key = String{keyStr};
    }

    return std::make_unique<ast::Sort>(std::move(targetExpr), key);
}

std::unique_ptr<ast::Match>
ASTConverter::convertMatch(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::MATCH) {
        throw std::runtime_error("Expected MATCH node");
    }

    // match expr { true => { ... }, false => { ... } }
    // first child is the target, rest are match_entry nodes
    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount < 1) {
        throw std::runtime_error("MATCH should have at least 1 child (target)");
    }

    TSNode targetNode = ts_node_named_child(node, 0);
    std::unique_ptr<ast::Expression> targetExpr = convertExpression(src, targetNode);

    std::vector<ast::Match::Candidate> candidates;

    // go through each case
    for (uint32_t i = 1; i < childCount; ++i) {
        TSNode entryNode = ts_node_named_child(node, i);

        if (ts_node_symbol(entryNode) != NodeType::MATCH_ENTRY) {
            continue;
        }

        // each case has a guard (the condition) and a body (the statements)
        uint32_t entryChildCount = ts_node_named_child_count(entryNode);
        if (entryChildCount != 2) {
            throw std::runtime_error("MATCH_ENTRY should have 2 children (guard and body)");
        }

        TSNode guardNode = ts_node_named_child(entryNode, 0);
        TSNode bodyNode = ts_node_named_child(entryNode, 1);

        std::unique_ptr<ast::Expression> guardExpr = convertExpression(src, guardNode);

        // parse all the statements in this case's body
        std::vector<std::unique_ptr<ast::Statement>> statements;

        if (ts_node_symbol(bodyNode) == NodeType::BODY) {
            uint32_t stmtCount = ts_node_named_child_count(bodyNode);
            for (uint32_t j = 0; j < stmtCount; ++j) {
                TSNode stmtNode = ts_node_named_child(bodyNode, j);

                auto stmt = convertStatement(src, stmtNode);
                statements.push_back(std::unique_ptr<ast::Statement>(
                    static_cast<ast::Statement*>(stmt.release())
                ));
            }
        }

        ast::Match::Candidate candidate{
            std::move(guardExpr),
            std::move(statements)
        };
        candidates.push_back(std::move(candidate));
    }

    return std::make_unique<ast::Match>(std::move(targetExpr), std::move(candidates));
}

std::unique_ptr<ast::InputChoice>
ASTConverter::convertInputChoice(const std::string &src, TSNode node) {
    // input_choice has 4 named children: player, prompt, choices, target
    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount < 4) {
        throw std::runtime_error("InputChoice requires 4 children");
    }

    TSNode playerNode = ts_node_named_child(node, 0);
    TSNode promptNode = ts_node_named_child(node, 1);
    TSNode choicesNode = ts_node_named_child(node, 2);
    TSNode targetNode = ts_node_named_child(node, 3);

    auto playerExpr = convertExpression(src, playerNode);
    auto choicesExpr = convertExpression(src, choicesNode);
    auto targetExpr = convertExpression(src, targetNode);

    String prompt = extractPromptString(src, promptNode);

    return ast::makeInputChoice(
        std::unique_ptr<ast::Variable>(static_cast<ast::Variable*>(playerExpr.release())),
        std::move(targetExpr),
        prompt,
        std::move(choicesExpr)
    );
}

std::unique_ptr<ast::InputText>
ASTConverter::convertInputText(const std::string &src, TSNode node) {
    // input_text has 3 named children: player, prompt, target
    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount < 3) {
        throw std::runtime_error("InputText requires 3 children");
    }

    TSNode playerNode = ts_node_named_child(node, 0);
    TSNode promptNode = ts_node_named_child(node, 1);
    TSNode targetNode = ts_node_named_child(node, 2);

    auto playerExpr = convertExpression(src, playerNode);
    auto targetExpr = convertExpression(src, targetNode);

    String prompt = extractPromptString(src, promptNode);

    return ast::makeInputText(
        std::unique_ptr<ast::Variable>(static_cast<ast::Variable*>(playerExpr.release())),
        std::move(targetExpr),
        prompt
    );
}

std::unique_ptr<ast::InputRange>
ASTConverter::convertInputRange(const std::string &src, TSNode node) {
    // input_range has 5 named children: player, prompt, min, max, target
    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount < 5) {
        throw std::runtime_error("InputRange requires 5 children");
    }

    TSNode playerNode = ts_node_named_child(node, 0);
    TSNode promptNode = ts_node_named_child(node, 1);
    TSNode minNode = ts_node_named_child(node, 2);
    TSNode maxNode = ts_node_named_child(node, 3);
    TSNode targetNode = ts_node_named_child(node, 4);

    auto playerExpr = convertExpression(src, playerNode);
    auto minExpr = convertExpression(src, minNode);
    auto maxExpr = convertExpression(src, maxNode);
    auto targetExpr = convertExpression(src, targetNode);

    String prompt = extractPromptString(src, promptNode);

    return ast::makeInputRange(
        std::unique_ptr<ast::Variable>(static_cast<ast::Variable*>(playerExpr.release())),
        std::move(targetExpr),
        prompt,
        std::move(minExpr),
        std::move(maxExpr)
    );
}

std::unique_ptr<ast::InputVote>
ASTConverter::convertInputVote(const std::string &src, TSNode node) {
    // input_vote has 4 named children: player, prompt, choices, target
    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount < 4) {
        throw std::runtime_error("InputVote requires 4 children");
    }

    TSNode playerNode = ts_node_named_child(node, 0);
    TSNode promptNode = ts_node_named_child(node, 1);
    TSNode choicesNode = ts_node_named_child(node, 2);
    TSNode targetNode = ts_node_named_child(node, 3);

    auto playerExpr = convertExpression(src, playerNode);
    auto choicesExpr = convertExpression(src, choicesNode);
    auto targetExpr = convertExpression(src, targetNode);

    String prompt = extractPromptString(src, promptNode);

    return ast::makeInputVote(
        std::unique_ptr<ast::Variable>(static_cast<ast::Variable*>(playerExpr.release())),
        std::move(targetExpr),
        prompt,
        std::move(choicesExpr)
    );
}

// unsupported statements - stubs
std::unique_ptr<ast::Statement>
ASTConverter::convertForLoop(const std::string &src, TSNode node) {
    TSSymbol symbol = ts_node_symbol(node);

    if (symbol != NodeType::FOR) {
        throw std::runtime_error("Expected FOR node");
    }

    uint32_t childCount = ts_node_named_child_count(node);
    if (childCount != 3) {
        throw std::runtime_error("FOR should have 3 named children (element, target, body)");
    }

    // Child 0: identifier (loop variable)
    TSNode elementNode = ts_node_named_child(node, 0);
    std::string elementName = extractText(src, elementNode);
    auto elementVar = std::make_unique<ast::Variable>(Name{elementName});

    // Child 1: expression (target list)
    TSNode targetNode = ts_node_named_child(node, 1);
    auto targetExpr = convertExpression(src, targetNode);

    // Child 2: body (loop body statements)
    TSNode bodyNode = ts_node_named_child(node, 2);
    std::vector<std::unique_ptr<ast::Statement>> statements;

    if (ts_node_symbol(bodyNode) == NodeType::BODY) {
        uint32_t stmtCount = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < stmtCount; ++i) {
            TSNode stmtNode = ts_node_named_child(bodyNode, i);
            auto stmt = convertStatement(src, stmtNode);
            statements.push_back(std::move(stmt));
        }
    }

    return ast::makeForLoop(std::move(elementVar), std::move(targetExpr), std::move(statements));
}

std::unique_ptr<ast::Statement>
ASTConverter::convertParallelFor(const std::string &src, TSNode node) {
    throw std::runtime_error("ParallelFor not supported by interpreter yet");
}

std::unique_ptr<ast::Statement>
ASTConverter::convertMessage(const std::string &src, TSNode node) {
    throw std::runtime_error("Message not supported by interpreter yet");
}

std::unique_ptr<ast::Statement>
ASTConverter::convertScores(const std::string &src, TSNode node) {
    throw std::runtime_error("Scores not supported by interpreter yet");
}

std::unique_ptr<ast::Statement>
ASTConverter::convertComment(const std::string &src, TSNode node) {
    throw std::runtime_error("Comment not supported by interpreter yet");
}
