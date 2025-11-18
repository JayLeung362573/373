#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

class Structure;

// Helper trait to check if a type is equality comparable
template<typename T, typename = void>
struct is_equality_comparable : std::false_type {};

template<typename T>
struct is_equality_comparable<T, std::void_t<
    decltype(std::declval<const T&>() == std::declval<const T&>())
>> : std::true_type {};

template<typename T>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

// Structure class - a heterogeneous container with value semantics
class Structure {
private:
    // Abstract base class for type-erased elements
    struct ElementBase {
        virtual ~ElementBase() = default;
        virtual std::unique_ptr<ElementBase> clone() const = 0;
        virtual void print(std::ostream& out) const = 0;
        virtual bool equals(const ElementBase& other) const = 0;
        virtual const void* get_type_id() const = 0;
    };

    // Template implementation for concrete element types
    template<typename T>
    struct ElementImpl : ElementBase {
        T value;

        explicit ElementImpl(const T& v) : value(v) {}

        std::unique_ptr<ElementBase> clone() const override {
            return std::make_unique<ElementImpl<T>>(value);
        }

        void print(std::ostream& out) const override {
            printElement(out, value);
        }

        bool equals(const ElementBase& other) const override {
            // Check if types match using type-specific static variable
            if (get_type_id() != other.get_type_id()) {
                return false;
            }
            // Safe to static_cast since we've verified the type
            const ElementImpl<T>* other_ptr = static_cast<const ElementImpl<T>*>(&other);
            return value == other_ptr->value;
        }

        const void* get_type_id() const override {
            // Each template instantiation gets its own static variable with unique address
            static const char type_id_var = 0;
            return &type_id_var;
        }
    };

    std::vector<std::unique_ptr<ElementBase>> elements;

public:
    // Default constructor
    Structure() = default;

    // Copy constructor - performs deep copy
    Structure(const Structure& other);

    // Copy assignment operator - performs deep copy
    Structure& operator=(const Structure& other);

    // Move constructor and assignment (defaulted)
    Structure(Structure&&) = default;
    Structure& operator=(Structure&&) = default;

    // Add an element to the structure
    // Special case: if T is int and value is 57, add it twice
    template<typename T>
    void add(const T& value) {
        static_assert(std::is_copy_constructible_v<T>, "Type must be copyable");
        static_assert(is_equality_comparable_v<T>, "Type must be equality comparable");

        elements.push_back(std::make_unique<ElementImpl<T>>(value));

        // Special case for integer value 57
        if constexpr (std::is_same_v<T, int>) {
            if (value == 57) {
                elements.push_back(std::make_unique<ElementImpl<T>>(value));
            }
        }
    }

    // Print all elements with "BETWEEN\n" separator
    void print(std::ostream& out) const;

    // Equality comparison
    // Returns false if either structure has more than 13 elements
    bool operator==(const Structure& other) const;
};

// printElement overload for Structure (definition in task01.cpp)
void printElement(std::ostream& out, const Structure& s);
