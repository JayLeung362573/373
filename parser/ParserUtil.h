#pragma once
#include <string>
#include <tree_sitter/api.h>

// Extract text substring from source based on node position
std::string slice(const std::string& src, TSNode node);

// Parse an integer from a node
int parseInteger(const std::string& src, TSNode node);

// Parse a quoted string and remove the quotes
std::string parseQuotedString(const std::string& src, TSNode node);

// Parse a boolean value
bool parseBoolean(const std::string& src, TSNode node);
