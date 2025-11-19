#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

class Structure;

void printElement(std::ostream& out, const Structure& s);

// Helper: Check if type T supports operator==
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
    struct Element {
        virtual ~Element() = default;
        virtual std::unique_ptr<Element> clone() const = 0;
        virtual void print(std::ostream& out) const = 0;
        virtual bool equals(const Element& other) const = 0;
        virtual const void* type_address() const = 0;  // Get unique type ID
    };

    template<typename T>
    struct ElementImpl : Element {
        T data;

        explicit ElementImpl(const T& value) : data(value) {}

        std::unique_ptr<Element> clone() const override {
            return std::make_unique<ElementImpl<T>>(data);
        }

        void print(std::ostream& out) const override {
            printElement(out, data);
        }

        bool equals(const Element& other) const override {
            // Check if types match by comparing static variable addresses
            if (type_address() != other.type_address()) {
                return false;  // Different types
            }
            // Same type - safe to cast and compare values
            const ElementImpl<T>* other_ptr = static_cast<const ElementImpl<T>*>(&other);
            return data == other_ptr->data;
        }

        const void* type_address() const override {
            // Each ElementImpl<T> instantiation gets its own static variable
            // Return its address as a unique type identifier
            static const char type_id = 0;
            return &type_id;
        }
    };

    std::vector<std::unique_ptr<Element>> elements;

public:
    Structure() = default;

    Structure(const Structure& other) {
        for (const auto& elem : other.elements) {
            elements.push_back(elem->clone());
        }
    }

    Structure& operator=(const Structure& other) {
        if (this != &other) {
            elements.clear();
            for (const auto& elem : other.elements) {
                elements.push_back(elem->clone());
            }
        }
        return *this;
    }

    Structure(Structure&&) = default;
    Structure& operator=(Structure&&) = default;

    template<typename T>
    void add(const T& value) {
        static_assert(std::is_copy_constructible_v<T>,
                      "Type must be copyable");
        static_assert(is_equality_comparable_v<T>,
                      "Type must be equality comparable");

        elements.push_back(std::make_unique<ElementImpl<T>>(value));

        // Special case: int value 57 is added twice
        if constexpr (std::is_same_v<T, int>) {
            if (value == 57) {
                elements.push_back(std::make_unique<ElementImpl<T>>(value));
            }
        }
    }

    void print(std::ostream& out) const;
    bool operator==(const Structure& other) const;
};

void printElement(std::ostream& out, const Structure& s);
