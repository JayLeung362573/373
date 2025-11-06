#pragma once
#include <string>
#include <tree_sitter/api.h>

namespace NodeType {
    inline TSSymbol INTEGER;
    inline TSSymbol BOOLEAN;
    inline TSSymbol QUOTED_STRING;
    inline TSSymbol NUMBER_RANGE;
    inline TSSymbol CONFIGURATION;
    inline TSSymbol CONFIGURATION_BLOCK;
    inline TSSymbol JSON_OBJECT;
    inline TSSymbol SETUP_BLOCK;
    inline TSSymbol CONSTANTS;
    inline TSSymbol VARIABLES;

    inline void init(const TSLanguage* language) {
        INTEGER = ts_language_symbol_for_name(language, "integer", 7, true);
        BOOLEAN = ts_language_symbol_for_name(language, "boolean", 7, true);
        QUOTED_STRING = ts_language_symbol_for_name(language, "quoted_string", 13, true);
        NUMBER_RANGE = ts_language_symbol_for_name(language, "number_range", 12, true);
        CONFIGURATION = ts_language_symbol_for_name(language, "configuration", 13, true);
        CONFIGURATION_BLOCK = ts_language_symbol_for_name(language, "configuration_block", 19, true);
        JSON_OBJECT = ts_language_symbol_for_name(language, "json_object", 11, true);
        SETUP_BLOCK = ts_language_symbol_for_name(language, "setup_block", 11, true);
        CONSTANTS = ts_language_symbol_for_name(language, "constants", 9, true);
        VARIABLES = ts_language_symbol_for_name(language, "variables", 9, true);
    }
}


