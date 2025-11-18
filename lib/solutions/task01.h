#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

class Structure;

// Check if type T supports operator==
template<typename T, typename = void>
struct is_equality_comparable : std::false_type {};

template<typename T>
struct is_equality_comparable<T, std::void_t<
    decltype(std::declval<const T&>() == std::declval<const T&>())
>> : std::true_type {};

template<typename T>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

class Structure {
private:
    // Base class for storing any type (type erasure)
    struct ElementBase {
        virtual ~ElementBase() = default;
        virtual std::unique_ptr<ElementBase> clone() const = 0;
        virtual void print(std::ostream& out) const = 0;
        virtual bool equals(const ElementBase& other) const = 0;
        virtual const void* get_type_id() const = 0;
    };

    // Concrete wrapper for a value of type T
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
            if (get_type_id() != other.get_type_id()) {
                return false;
            }
            const ElementImpl<T>* other_ptr = static_cast<const ElementImpl<T>*>(&other);
            return value == other_ptr->value;
        }

        // Each ElementImpl<T> has its own static variable
        // We use its address as a unique type ID (avoids dynamic_cast)
        const void* get_type_id() const override {
            static const char type_id = 0;
            return &type_id;
        }
    };

    std::vector<std::unique_ptr<ElementBase>> elements;

public:
    Structure() = default;
    Structure(const Structure& other);
    Structure& operator=(const Structure& other);
    Structure(Structure&&) = default;
    Structure& operator=(Structure&&) = default;

    // Add element to Structure
    // Special case: int value 57 is added twice
    template<typename T>
    void add(const T& value) {
        static_assert(std::is_copy_constructible_v<T>, "Type must be copyable");
        static_assert(is_equality_comparable_v<T>, "Type must be equality comparable");

        elements.push_back(std::make_unique<ElementImpl<T>>(value));

        // Special case: 57 gets added twice
        if constexpr (std::is_same_v<T, int>) {
            if (value == 57) {
                elements.push_back(std::make_unique<ElementImpl<T>>(value));
            }
        }
    }

    void print(std::ostream& out) const;
    bool operator==(const Structure& other) const;
};

// Allows Structures to contain other Structures
void printElement(std::ostream& out, const Structure& s);
