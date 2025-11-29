#include "GameSpecLoader.h"
#include "ASTConverter.h"
#include <tree_sitter/api.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <spdlog/spdlog.h>


//for now we will just parse the config
extern "C" const TSLanguage *tree_sitter_socialgaming();
//for tree-sitters TSLLanguage structure, it's rules etc.
//tree_sitter_socialgaming() will generate when you run "tree-sitter generate" from the terminal

// Read a file into memory as a single string
static std::string slurp(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot open file: " + path);

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
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
            spdlog::error("Parse failed: {}", path);
            ts_parser_delete(parser);
            return false;
        }

        TSNode root = ts_tree_root_node(tree);
        spdlog::info("Parsed OK: {} | root ={} | children={}", path, ts_node_type(root), ts_node_child_count(root));

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
                spdlog::info("Configuration snippet:\n{}", snippet);
            }
        }
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return true;
    } catch (const std::exception &e) {
        spdlog::error("HelloWorldSmokeTest error: {}", e.what());
        return false;
    } catch (...) {
        spdlog::error("HelloWorldSmokeTest error: unknown exception");
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
        } else if (strcmp(field_name, "rules") == 0 || ts_node_symbol(child) == NodeType::RULES) {
            parseRules(text, child, spec);
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
    // TODO: Implement parsing for setup rules based on field names
    (void)src;
    (void)node;
    (void)spec;
}

void GameSpecLoader::parseRules(const std::string &src, TSNode node, GameSpec &spec) {
    // The rules node might contain a body node, so we need to check
    uint32_t child_count = ts_node_named_child_count(node);

    std::cout << "\nPARSING RULES AND BUILDING AST" << std::endl;

    // If there's only one child and it's a body, parse that instead.
    // This is specified in the grammar, it's a hidden node that comes after the rules bit
    TSNode statementsNode = node;
    if (child_count == 1) {
        TSNode firstChild = ts_node_named_child(node, 0);
        if (ts_node_symbol(firstChild) == NodeType::BODY) {
            std::cout << "Rules contains a body node, parsing its contents" << std::endl;
            statementsNode = firstChild;
            child_count = ts_node_named_child_count(statementsNode);
        }
    }

    std::cout << "Found " << child_count << " statements" << std::endl;

    // Convert each statement to AST
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_named_child(statementsNode, i);

        try {
            std::cout << "[Statement " << i << "] Converting to AST" << std::endl;
            auto astNode = ASTConverter::convertStatement(src, child);
            spec.rulesProgram.push_back(std::unique_ptr<ast::Statement>(
                static_cast<ast::Statement*>(astNode.release())
            ));
        } catch (const std::exception& e) {
            std::cerr << "  Error converting statement: " << e.what() << std::endl;
        }
    }

    std::cout << "Built AST with " << spec.rulesProgram.size() << " statements" << std::endl;
    std::cout << "END PARSING RULES\n" << std::endl;
}