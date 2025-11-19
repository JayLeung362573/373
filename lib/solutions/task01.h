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
public:
    Structure() = default;

    Structure(const Structure& other) {
        elements.reserve(other.elements.size());
        for(const auto& element : other.elements) {
            elements.push_back(element->clone());
        }
    }

    Structure& operator=(const Structure& other) {
        if(this != &other) {
            elements.clear();
            elements.reserve(other.elements.size());
            for(const auto& element : other.elements) {
                elements.push_back(element->clone());
            }
        }
        return *this;
    }

    template<typename T>
    void add(const T& value) {
        // Compile-time checks
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

    bool operator==(const Structure& other) const;
    void print(std::ostream& out) const;

private:
    struct Element {
        virtual ~Element() = default;
        virtual std::unique_ptr<Element> clone() const = 0;
        virtual void print(std::ostream& out) const = 0;
        virtual bool equals(const Element& other) const = 0;
        virtual const void* get_type_id() const = 0;  // Simpler: just void*
    };

    template<typename T>
    struct ElementImpl : Element {
        T data;

        ElementImpl(const T& value) : data(value) {}

        std::unique_ptr<Element> clone() const override {
            return std::make_unique<ElementImpl<T>>(data);
        }

        void print(std::ostream& out) const override {
            printElement(out, data);
        }

        // SIMPLEST VERSION: Each ElementImpl<T> has its own static variable
        // We return its address as a unique identifier
        const void* get_type_id() const override {
            static const char type_id = 0;  // Unique per ElementImpl<T>
            return &type_id;
        }

        bool equals(const Element& other) const override {
            // Check if same type by comparing addresses
            if(get_type_id() != other.get_type_id()) {
                return false;  // Different types
            }
            // Same type - safe to cast and compare
            const auto& otherImpl = static_cast<const ElementImpl<T>&>(other);
            return data == otherImpl.data;
        }
    };

    std::vector<std::unique_ptr<Element>> elements;
};
