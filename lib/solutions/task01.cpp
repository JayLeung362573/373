#include "task01.h"

// Step 3: Print all elements with "BETWEEN\n" separator
void Structure::print(std::ostream& out) const {
    bool first = true;
    for (const auto& elem : elements) {
        if (!first) {
            out << "BETWEEN\n";
        }
        elem->print(out);
        out << '\n';
        first = false;
    }
}

// Step 4: Equality comparison
// Returns false if either Structure has more than 13 elements
bool Structure::operator==(const Structure& other) const {
    if (elements.size() > 13 || other.elements.size() > 13) {
        return false;
    }
    if (elements.size() != other.elements.size()) {
        return false;
    }
    for (size_t i = 0; i < elements.size(); ++i) {
        if (!elements[i]->equals(*other.elements[i])) {
            return false;
        }
    }
    return true;
}

// Step 3: printElement for Structure (enables nested Structures)
void printElement(std::ostream& out, const Structure& s) {
    s.print(out);
}
