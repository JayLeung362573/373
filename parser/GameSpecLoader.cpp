#include "GameSpecLoader.h"
#include <tree_sitter/api.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <iostream>


//for now we will just parse the config
extern "C" const TSLanguage *tree_sitter_socialgaming();
//for tree-sitters TSLLanguage structure, it's rules etc.
//tree_sitter_socialgaming() will generate when you run "tree-sitter generate" from the terminal

/*
 * Read a file into memory as a single string
 */
static std::string slurp(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file: " + path);

    std::ostringstream ss;
    ss << in.rdbuf(); // read entire file stream
    return ss.str(); // return as str

    /*
     ex: const std::string src = slurp("rock-paper-scissors.game");
     - This will read the files text into memory and lets Tree-Sitter parse it.
     */
}

/*
 * Take the full source text and the TSNode (we has start/end indices) and extract the substring that node has
 */
static std::string slice(const std::string &src, TSNode node) {
    auto start = ts_node_start_byte(node);
    auto end = ts_node_end_byte(node);
    auto size = static_cast<uint32_t>(src.size());

    //make sure it's valid range
    if (start >= size || end <= start) return {}; // returns an empty string

    return {src.begin() + start, src.begin() + end};

}

static int parseInteger(const std::string &src, TSNode node) {
    std::string numStr = slice(src, node);
    return std::stoi(numStr);
}

static std::string parseQuotedString(const std::string &src, TSNode node) {
    std::string withQuotes = slice(src, node);
    if (withQuotes.size() >= 2) {
        return withQuotes.substr(1, withQuotes.size() - 2);
    }
    return withQuotes;
}

static bool parseBoolean(const std::string &src, TSNode node) {
    std::string boolStr = slice(src, node);
    return boolStr == "true";
}

//this will read the hello-test.game, and print out to the terminal the config snippet, for testing that it can see the config block as a node
bool GameSpecLoader::HelloWorldSmokeTest(const char *path) {
    try {
        //read file
        const std::string src = slurp(path);

        //parse with tree-sitter
        TSParser *parser = ts_parser_new();
        const TSLanguage* language = tree_sitter_socialgaming();
        ts_parser_set_language(parser, language);

        NodeType::init(language);

        TSTree *tree = ts_parser_parse_string(parser, nullptr, src.c_str(), static_cast<uint32_t>(src.size()));

        if (!tree) {
            std::cerr << "Parse failed: " << path << std::endl;
            ts_parser_delete(parser);
            return false;
        }

        TSNode root = ts_tree_root_node(tree);
        std::cout << "Parsed OK: " << path
                  << " | root =" << ts_node_type(root)
                  << " | children=" << ts_node_child_count(root)
                  << std::endl;

        //display the snippet of what the config file says so we can check if it parses
        uint32_t child_count = ts_node_child_count(root);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode ch = ts_node_child(root, i);
            if (ts_node_symbol(ch) == NodeType::CONFIGURATION ||
                ts_node_symbol(ch) == NodeType::CONFIGURATION_BLOCK) {
                std::string snippet = slice(src, ch);
                if (snippet.size() > 200) {
                    snippet.erase(200);
                    snippet.append("...");
                }
                std::cout << "\n Configuration snippet \n"
                          << snippet
                          << "\n";
            }
        }
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return true;
    } catch (const std::exception &e) {
        std::cerr << "HelloWorldSmokeTest error: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "HelloWorldSmokeTest error: unknown exception" << std::endl;
        return false;
    }

}

GameSpec GameSpecLoader::loadFile(const char *path) {
    const std::string src = slurp(path);
    return loadString(src);
}

GameSpec GameSpecLoader::loadString(const std::string &text) {
    GameSpec spec;

    //Parse using Tree-Sitter.
    TSParser *parser = ts_parser_new();
    const TSLanguage* language = tree_sitter_socialgaming();
    ts_parser_set_language(parser, language);

    NodeType::init(language);

    TSTree *tree = ts_parser_parse_string(parser, nullptr, text.c_str(), text.size());

    if (!tree) {
        ts_parser_delete(parser);
        throw std::runtime_error("Parse failed");
    }

    TSNode root = ts_tree_root_node(tree);

    //loop through the children to parse the config of each
    uint32_t child_count = ts_node_child_count(root);

    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(root, i);
        const char *field_name = ts_node_field_name_for_child(root, i);

        if (!field_name) {
            if (ts_node_symbol(child) == NodeType::CONFIGURATION) {
                parseConfiguration(text, child, spec);
            }
            continue;
        }

        //if it's config we will use a diff function because this can vary quite a bit.
        if (strcmp(field_name, "configuration") == 0 || ts_node_symbol(child) == NodeType::CONFIGURATION) {
            parseConfiguration(text, child, spec);
        } else if (strcmp(field_name, "constants") == 0 || ts_node_symbol(child) == NodeType::CONSTANTS) {
            spec.constants = slice(text, child);
        } else if (strcmp(field_name, "variables") == 0 || ts_node_symbol(child) == NodeType::VARIABLES) {
            spec.variables = slice(text, child);
        }
    }


    //no memory leaks :).
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    return spec;
}

void GameSpecLoader::parseConfiguration(const std::string &src, TSNode node, GameSpec &spec) {
    // Walk through children of the configuration block
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        const char* field_name = ts_node_field_name_for_child(node, i);

        if (!field_name) continue;

        // Parse based on field name, not just type
        if (strcmp(field_name, "name") == 0 && ts_node_symbol(child) == NodeType::QUOTED_STRING) {
            spec.name = parseQuotedString(src, child);
        }
        else if (strcmp(field_name, "player_count") == 0 && ts_node_symbol(child) == NodeType::NUMBER_RANGE) {
            parsePlayerRange(src, child, spec);
        }
        else if (strcmp(field_name, "audience") == 0 && ts_node_symbol(child) == NodeType::BOOLEAN) {
            spec.hasAudience = parseBoolean(src, child);
        }
        else if (strcmp(field_name, "setup") == 0 &&
                 (ts_node_symbol(child) == NodeType::JSON_OBJECT || ts_node_symbol(child) == NodeType::SETUP_BLOCK)) {
            parseSetup(src, child, spec);
        }
    }
}

void GameSpecLoader::parsePlayerRange(const std::string &src, TSNode node, GameSpec &spec) {
    // Player range is typically (min, max)
    uint32_t child_count = ts_node_child_count(node);

    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        const char* field_name = ts_node_field_name_for_child(node, i);

        if (!field_name && ts_node_symbol(child) != NodeType::INTEGER) continue;

        if (ts_node_symbol(child) == NodeType::INTEGER) {
            int value = parseInteger(src, child);

            // Use field name to determine min vs max, fallback to order
            if (field_name && strcmp(field_name, "min") == 0) {
                spec.playerRange.min = value;
            } else if (field_name && strcmp(field_name, "max") == 0) {
                spec.playerRange.max = value;
            } else {
                // Fallback: first integer is min, second is max
                if (spec.playerRange.min == 0) {
                    spec.playerRange.min = value;
                } else {
                    spec.playerRange.max = value;
                }
            }
        }
    }
}

void GameSpecLoader::parseSetup(const std::string &src, TSNode node, GameSpec &spec) {
    // For now, we'll just capture the setup rules without deep parsing
    uint32_t child_count = ts_node_child_count(node);

    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        const char* field_name = ts_node_field_name_for_child(node, i);

        if (!field_name) continue;

        // Look for setup rule definitions
        // Each rule will have fields like: id, kind, prompt, and optional range
        // TODO: Implement parsing for setup rules based on field names

    }
}


