#include "task01.h"

// printElement overload for Structure
// This allows Structures to contain other Structures hierarchically
void printElement(std::ostream& out, const Structure& s) {
    s.print(out);
}
