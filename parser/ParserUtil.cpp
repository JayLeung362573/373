#include "ParserUtil.h"
#include <iostream>
#include <stdexcept>

std::string slice(const std::string& src, TSNode node) {
    auto start = ts_node_start_byte(node);
    auto end = ts_node_end_byte(node);
    auto size = static_cast<uint32_t>(src.size());

    if (start >= size || end <= start || end > size) {
        std::cerr << "Warning: slice() bounds error - start=" << start
                  << " end=" << end << " size=" << size << std::endl;
        return {};
    }

    return {src.begin() + start, src.begin() + end};
}

int parseInteger(const std::string& src, TSNode node) {
    std::string numStr = slice(src, node);
    try {
        return std::stoi(numStr);
    } catch (const std::exception& e) {
        std::cerr << "Warning: parseInteger() failed for '" << numStr << "': " << e.what() << std::endl;
        return 0;
    }
}

std::string parseQuotedString(const std::string& src, TSNode node) {
    std::string withQuotes = slice(src, node);
    if (withQuotes.size() >= 2) {
        return withQuotes.substr(1, withQuotes.size() - 2);
    }
    return withQuotes;
}

bool parseBoolean(const std::string& src, TSNode node) {
    std::string boolStr = slice(src, node);
    return boolStr == "true";
}
