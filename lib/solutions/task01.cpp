#include "task01.h"

// Copy constructor - creates a deep copy of another Structure
Structure::Structure(const Structure& other) {
    for (const auto& elem : other.elements) {
        elements.push_back(elem->clone());
    }
}

// Copy assignment - replaces contents with a deep copy of another Structure
Structure& Structure::operator=(const Structure& other) {
    if (this != &other) {
        elements.clear();
        for (const auto& elem : other.elements) {
            elements.push_back(elem->clone());
        }
    }
    return *this;
}

// Print all elements with "BETWEEN\n" between them
void Structure::print(std::ostream& out) const {
    bool first = true;
    for (const auto& elem : elements) {
        // Print separator before all elements except the first
        if (!first) {
            out << "BETWEEN\n";
        }
        // Print the element (delegates to printElement via virtual dispatch)
        elem->print(out);
        // Print newline after each element
        out << '\n';
        first = false;
    }
}

// Check if two Structures are equal
// Returns false if either Structure has more than 13 elements
bool Structure::operator==(const Structure& other) const {
    // Structures with more than 13 elements are never equal
    if (elements.size() > 13 || other.elements.size() > 13) {
        return false;
    }
    // Different sizes means not equal
    if (elements.size() != other.elements.size()) {
        return false;
    }
    // Check each element for equality
    for (size_t i = 0; i < elements.size(); ++i) {
        if (!elements[i]->equals(*other.elements[i])) {
            return false;
        }
    }
    return true;
}

// printElement overload for Structure
// Allows Structures to contain other Structures (hierarchical/recursive structure)
void printElement(std::ostream& out, const Structure& s) {
    s.print(out);
}
