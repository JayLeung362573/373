#pragma once

#include <memory>
#include <string>
#include <tree_sitter/api.h>
#include "src/GameEngine/Rules.h"
#include "src/GameEngine/Types.h"

// Converts Tree-Sitter nodes to AST nodes
class ASTConverter {
public:
    // Convert expression nodes
    static std::unique_ptr<ast::Expression>
    convertExpression(const std::string& src, TSNode node);

    // Convert statement nodes
    static std::unique_ptr<ast::Statement>
    convertStatement(const std::string& src, TSNode node);

    // Statement conversions
    static std::unique_ptr<ast::Assignment>
    convertAssignment(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Discard>
    convertDiscard(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Extend>
    convertExtend(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Reverse>
    convertReverse(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Shuffle>
    convertShuffle(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Sort>
    convertSort(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Match>
    convertMatch(const std::string& src, TSNode node);

    static std::unique_ptr<ast::InputChoice>
    convertInputChoice(const std::string& src, TSNode node);

    static std::unique_ptr<ast::InputText>
    convertInputText(const std::string& src, TSNode node);

    static std::unique_ptr<ast::InputRange>
    convertInputRange(const std::string& src, TSNode node);

    static std::unique_ptr<ast::InputVote>
    convertInputVote(const std::string& src, TSNode node);

    // unsupported statements (stub converters - need interpreter AST nodes)
    static std::unique_ptr<ast::Statement>
    convertForLoop(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Statement>
    convertParallelFor(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Statement>
    convertMessage(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Statement>
    convertScores(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Statement>
    convertComment(const std::string& src, TSNode node);

private:
    // Expression conversions
    static std::unique_ptr<ast::Constant>
    convertConstant(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Variable>
    convertVariable(const std::string& src, TSNode node);

    static std::unique_ptr<ast::Attribute>
    convertQualifiedIdentifier(const std::string& src, TSNode node);

    // Value conversions for constants
    static Value convertValue(const std::string& src, TSNode node);
    static Value convertListLiteral(const std::string& src, TSNode node);
    static Value convertValueMap(const std::string& src, TSNode node);

    // Utilities
    static std::string extractText(const std::string& src, TSNode node);
    static std::string extractQuotedString(const std::string& src, TSNode node);
};
