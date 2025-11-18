#include "task01.h"

// Copy constructor - deep copy all elements
Structure::Structure(const Structure& other) {
    for (const auto& elem : other.elements) {
        elements.push_back(elem->clone());
    }
}

// Copy assignment - deep copy all elements
Structure& Structure::operator=(const Structure& other) {
    if (this != &other) {
        elements.clear();
        for (const auto& elem : other.elements) {
            elements.push_back(elem->clone());
        }
    }
    return *this;
}

// Print all elements with "BETWEEN\n" separator
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

// Check equality
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

// printElement for Structure (enables nested Structures)
void printElement(std::ostream& out, const Structure& s) {
    s.print(out);
}
