#include "task01.h"

void printElement(std::ostream& out, const Structure& s) {
    s.print(out);
}

void Structure::print(std::ostream& out) const {
    bool first = true;
    for(const auto& element : elements) {
        if(!first) {
            out << "BETWEEN\n";
        }
        element->print(out);
        out << '\n';
        first = false;
    }
}

bool Structure::operator==(const Structure& other) const {
    // Returns false if either Structure has more than 13 elements
    if(elements.size() > 13 || other.elements.size() > 13) {
        return false;
    }

    if(elements.size() != other.elements.size()) {
        return false;
    }

    for(size_t i = 0; i < elements.size(); ++i) {
        if(!elements[i]->equals(*other.elements[i])) {
            return false;
        }
    }
    return true;
}
