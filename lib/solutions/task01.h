#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

class Structure;

// Type trait to check if a type supports equality comparison (operator==)
template<typename T, typename = void>
struct is_equality_comparable : std::false_type {};

template<typename T>
struct is_equality_comparable<T, std::void_t<
    decltype(std::declval<const T&>() == std::declval<const T&>())
>> : std::true_type {};

template<typename T>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

// Structure - a heterogeneous container that can hold different types
// Uses type erasure pattern: ElementBase (abstract) + ElementImpl<T> (concrete)
class Structure {
private:
    // Base class for type-erased storage
    // Defines the interface that all stored elements must support
    struct ElementBase {
        virtual ~ElementBase() = default;

        // Create a deep copy of this element
        virtual std::unique_ptr<ElementBase> clone() const = 0;

        // Print this element using the appropriate printElement overload
        virtual void print(std::ostream& out) const = 0;

        // Check equality with another element
        virtual bool equals(const ElementBase& other) const = 0;

        // Get a unique type identifier (avoids dynamic_cast)
        virtual const void* get_type_id() const = 0;
    };

    // Template wrapper that stores a value of type T
    // Implements the ElementBase interface for any copyable, equality-comparable type
    template<typename T>
    struct ElementImpl : ElementBase {
        T value;

        explicit ElementImpl(const T& v) : value(v) {}

        // Deep copy by creating a new ElementImpl with a copy of the value
        std::unique_ptr<ElementBase> clone() const override {
            return std::make_unique<ElementImpl<T>>(value);
        }

        // Delegates to the appropriate printElement overload for type T
        void print(std::ostream& out) const override {
            printElement(out, value);
        }

        // Type-safe equality check without dynamic_cast
        bool equals(const ElementBase& other) const override {
            // First check if types match
            if (get_type_id() != other.get_type_id()) {
                return false;
            }
            // Types match, safe to static_cast and compare values
            const ElementImpl<T>* other_ptr = static_cast<const ElementImpl<T>*>(&other);
            return value == other_ptr->value;
        }

        // Returns address of a type-specific static variable as unique identifier
        // Each ElementImpl<T> instantiation has its own static variable
        const void* get_type_id() const override {
            static const char type_id = 0;
            return &type_id;
        }
    };

    // Container of type-erased elements
    std::vector<std::unique_ptr<ElementBase>> elements;

public:
    // Default constructor creates an empty Structure
    Structure() = default;

    // Deep copy constructor
    Structure(const Structure& other);

    // Deep copy assignment
    Structure& operator=(const Structure& other);

    // Move operations (efficient transfer of ownership)
    Structure(Structure&&) = default;
    Structure& operator=(Structure&&) = default;

    // Add an element to the Structure
    // Requires: T must be copyable and equality-comparable
    //           printElement(std::ostream&, const T&) must be declared
    // Special case: int value 57 is added twice
    template<typename T>
    void add(const T& value) {
        // Compile-time checks for required operations
        static_assert(std::is_copy_constructible_v<T>,
                      "Type must be copyable");
        static_assert(is_equality_comparable_v<T>,
                      "Type must be equality comparable");

        // Store a copy of the value
        elements.push_back(std::make_unique<ElementImpl<T>>(value));

        // Special case: 57 gets added twice
        if constexpr (std::is_same_v<T, int>) {
            if (value == 57) {
                elements.push_back(std::make_unique<ElementImpl<T>>(value));
            }
        }
    }

    // Print all elements, separated by "BETWEEN\n"
    void print(std::ostream& out) const;

    // Compare two Structures for equality
    // Returns false if either has more than 13 elements
    bool operator==(const Structure& other) const;
};

// printElement overload for Structure (allows nested Structures)
void printElement(std::ostream& out, const Structure& s);
