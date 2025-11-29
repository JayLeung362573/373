#pragma once
#include <tree_sitter/api.h>

// tree-sitter grammar symbols for the SocialGaming language
// these map to node types in the CST, not our AST
namespace NodeType {
    // primitives
    inline TSSymbol INTEGER;
    inline TSSymbol BOOLEAN;
    inline TSSymbol QUOTED_STRING;
    inline TSSymbol NUMBER_RANGE;
    inline TSSymbol IDENTIFIER;

    // game file sections
    inline TSSymbol CONFIGURATION;
    inline TSSymbol CONFIGURATION_BLOCK;
    inline TSSymbol CONSTANTS;
    inline TSSymbol VARIABLES;
    inline TSSymbol RULES;
    inline TSSymbol SETUP_BLOCK;

    // structural
    inline TSSymbol BODY;
    inline TSSymbol RULE;
    inline TSSymbol JSON_OBJECT;
    inline TSSymbol VALUE_MAP;
    inline TSSymbol LIST_LITERAL;

    // expressions
    inline TSSymbol EXPRESSION;
    inline TSSymbol QUALIFIED_IDENTIFIER;

    // statements
    inline TSSymbol ASSIGNMENT;
    inline TSSymbol DISCARD;
    inline TSSymbol EXTEND;
    inline TSSymbol REVERSE;
    inline TSSymbol SHUFFLE;
    inline TSSymbol SORT;
    inline TSSymbol MATCH;
    inline TSSymbol MATCH_ENTRY;

    // input statements
    inline TSSymbol INPUT_CHOICE;
    inline TSSymbol INPUT_TEXT;
    inline TSSymbol INPUT_RANGE;
    inline TSSymbol INPUT_VOTE;

    // control flow (not yet supported by interpreter)
    inline TSSymbol FOR;
    inline TSSymbol PARALLEL_FOR;
    inline TSSymbol MESSAGE;
    inline TSSymbol SCORES;
    inline TSSymbol COMMENT;

    // method calls / builtins (not yet supported by interpreter)
    inline TSSymbol BUILTIN;
    inline TSSymbol ARGUMENT_LIST;
    inline TSSymbol EXPRESSION_LIST;

    // other stuff
    inline TSSymbol PLAYER_SET;
    inline TSSymbol STRING_INTERPOLATION;

    inline void init(const TSLanguage* language) {
        INTEGER = ts_language_symbol_for_name(language, "integer", 7, true);
        BOOLEAN = ts_language_symbol_for_name(language, "boolean", 7, true);
        QUOTED_STRING = ts_language_symbol_for_name(language, "quoted_string", 13, true);
        NUMBER_RANGE = ts_language_symbol_for_name(language, "number_range", 12, true);
        IDENTIFIER = ts_language_symbol_for_name(language, "identifier", 10, true);

        CONFIGURATION = ts_language_symbol_for_name(language, "configuration", 13, true);
        CONFIGURATION_BLOCK = ts_language_symbol_for_name(language, "configuration_block", 19, true);
        CONSTANTS = ts_language_symbol_for_name(language, "constants", 9, true);
        VARIABLES = ts_language_symbol_for_name(language, "variables", 9, true);
        RULES = ts_language_symbol_for_name(language, "rules", 5, true);
        SETUP_BLOCK = ts_language_symbol_for_name(language, "setup_block", 11, true);

        BODY = ts_language_symbol_for_name(language, "body", 4, true);
        RULE = ts_language_symbol_for_name(language, "rule", 4, true);
        JSON_OBJECT = ts_language_symbol_for_name(language, "json_object", 11, true);
        VALUE_MAP = ts_language_symbol_for_name(language, "value_map", 9, true);
        LIST_LITERAL = ts_language_symbol_for_name(language, "list_literal", 12, true);

        EXPRESSION = ts_language_symbol_for_name(language, "expression", 10, true);
        QUALIFIED_IDENTIFIER = ts_language_symbol_for_name(language, "qualified_identifier", 20, true);

        ASSIGNMENT = ts_language_symbol_for_name(language, "assignment", 10, true);
        DISCARD = ts_language_symbol_for_name(language, "discard", 7, true);
        EXTEND = ts_language_symbol_for_name(language, "extend", 6, true);
        REVERSE = ts_language_symbol_for_name(language, "reverse", 7, true);
        SHUFFLE = ts_language_symbol_for_name(language, "shuffle", 7, true);
        SORT = ts_language_symbol_for_name(language, "sort", 4, true);
        MATCH = ts_language_symbol_for_name(language, "match", 5, true);
        MATCH_ENTRY = ts_language_symbol_for_name(language, "match_entry", 11, true);

        INPUT_CHOICE = ts_language_symbol_for_name(language, "input_choice", 12, true);
        INPUT_TEXT = ts_language_symbol_for_name(language, "input_text", 10, true);
        INPUT_RANGE = ts_language_symbol_for_name(language, "input_range", 11, true);
        INPUT_VOTE = ts_language_symbol_for_name(language, "input_vote", 10, true);

        FOR = ts_language_symbol_for_name(language, "for", 3, true);
        PARALLEL_FOR = ts_language_symbol_for_name(language, "parallel_for", 12, true);
        MESSAGE = ts_language_symbol_for_name(language, "message", 7, true);
        SCORES = ts_language_symbol_for_name(language, "scores", 6, true);
        COMMENT = ts_language_symbol_for_name(language, "comment", 7, true);

        BUILTIN = ts_language_symbol_for_name(language, "builtin", 7, true);
        ARGUMENT_LIST = ts_language_symbol_for_name(language, "argument_list", 13, true);
        EXPRESSION_LIST = ts_language_symbol_for_name(language, "expression_list", 15, true);

        PLAYER_SET = ts_language_symbol_for_name(language, "player_set", 10, true);
        STRING_INTERPOLATION = ts_language_symbol_for_name(language, "string_interpolation", 20, true);
    }
}
