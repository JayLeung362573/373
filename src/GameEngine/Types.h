// Defines types used by the interpreter to store game state.

#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <stdexcept>
#include <ostream>
#include <algorithm>
#include <random>
#include <utility>

#include <iostream>

/// Represents a variable name in the AST.
/// Used as a key in variable lookups.
struct Name
{
    std::string name;

    bool operator==(const Name& other) const noexcept
    {
        return name == other.name;
    }
};

/// Represents a string value.
struct String
{
    std::string value;

    bool operator==(const String& other) const noexcept
    {
        return value == other.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const String& s)
    {
        os << '"' << s.value << '"';
        return os;
    }
};

/// Represents an integer value.
struct Integer
{
    int value;

    bool operator==(const Integer& other) const noexcept
    {
        return value == other.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Integer& integer)
    {
        os << integer.value;
        return os;
    }

    bool operator<(const Integer& other) const noexcept
    {
        return value < other.value;
    }
};

/// Represents an boolean value.
struct Boolean
{
    bool value;

    bool operator==(const Boolean& other) const noexcept
    {
        return value == other.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const Boolean& boolean)
    {
        os << boolean.value;
        return os;
    }
};

/// Hash specializations for Name and String so they can be used as keys in unordered_map.
namespace std
{
    template<>
    struct hash<Name>
    {
        size_t operator()(const Name& v) const noexcept
        {
            return std::hash<std::string>()(v.name);
        }
    };

    template<>
    struct hash<String>
    {
        size_t operator()(const String& s) const noexcept
        {
            return std::hash<std::string>()(s.value);
        }
    };
}

/// A generic list type used to store a list of Values.
template <typename T>
struct List
{
    std::vector<T> value;

    List() = default;
    List(std::initializer_list<T> init) : value(init) {}

    Integer size()
    {
        return Integer{(int)(value.size())};
    }

    void extend(const List<T>& list)
    {
        value.insert(value.end(), list.value.begin(), list.value.end());
    }

    void reverse()
    {
        std::reverse(value.begin(), value.end());
    }

    void shuffle()
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(value.begin(), value.end(), g);
    }

    /// Discards `amount` items starting from the start of the list (the top?)
    /// `amount` will be clamped to min(amount, list.size())
    /// A negative `amount` is a no-op
    void discard(Integer amount)
    {
        if (amount.value < 0)
        {
            return;
        }
        int clampedAmount = std::min(amount.value, size().value);

        value.erase(value.begin(), value.begin() + clampedAmount);
    }

    // Print example: [ "a", "b", "c" ]
    friend std::ostream& operator<<(std::ostream& os, const List<T>& list)
    {
        os << "[ ";
        auto it = list.value.begin();
        while (it != list.value.end())
        {
            os << *it;
            it++;
            if (it != list.value.end())
            {
                os << ", ";
            }
        }
        os << " ]";
        return os;
    }

    bool operator==(const List<T>& other) const noexcept
    {
        return value == other.value;
    }
};

/// A generic map type used to store String, Value pairs.
template <typename K, typename V>
struct Map
{
    std::unordered_map<K, V> value;

    /// Gets the value at an attribute as a reference.
    /// Throws if the attribute isn't set.
    const V& getAttribute(K attr) const
    {
        if (!value.contains(attr))
        {
            throw std::runtime_error("Attribute does not exist in Map");
        }
        return value.at(attr);
    }

    V& getAttribute(K attr)
    {
        return const_cast<V&>(std::as_const(*this).getAttribute(attr));
    }

    /// Sets or overwrites a named attribute.
    void setAttribute(K attr, V val)
    {
        value[attr] = val;
    }

    bool operator==(const Map<K, V>& other) const noexcept
    {
        return value == other.value;
    }

    // Print example: { "a": { "b": "c" } }
    friend std::ostream& operator<<(std::ostream& os, const Map<K, V>& map)
    {
        os << "{ ";
        auto it = map.value.begin();
        while (it != map.value.end())
        {
            os << it->first << ": " << it->second;
            it++;
            if (it != map.value.end())
            {
                os << ", ";
            }
        }
        os << " }";
        return os;
    }
};

/// Represents any value.
/// A value may be a List, Map, or String, and can be accessed
/// through the asString(), asMap(), asList() methods respectively.
struct Value
{
    std::variant<List<Value>, Map<String, Value>, String, Integer, Boolean> value;

    bool isString() const
    {
        return std::holds_alternative<String>(value);
    }

    bool isInteger() const
    {
        return std::holds_alternative<Integer>(value);
    }

    bool isBoolean() const
    {
        return std::holds_alternative<Boolean>(value);
    }

    bool isList() const
    {
        return std::holds_alternative<List<Value>>(value);
    }

    bool isMap() const
    {
        return std::holds_alternative<Map<String, Value>>(value);
    }

    const String& asString() const
    {
        if (!isString()) { throw std::runtime_error("Value is not a String"); }
        return std::get<String>(value);
    }

    String& asString()
    {
        return const_cast<String&>(std::as_const(*this).asString());
    }

    const Integer& asInteger() const
    {
        if (!isInteger()) { throw std::runtime_error("Value is not an Integer"); }
        return std::get<Integer>(value);
    }

    Integer& asInteger()
    {
        return const_cast<Integer&>(std::as_const(*this).asInteger());
    }

    const Boolean& asBoolean() const
    {
        if (!isBoolean()) { throw std::runtime_error("Value is not a Boolean"); }
        return std::get<Boolean>(value);
    }

    Boolean& asBoolean()
    {
        return const_cast<Boolean&>(std::as_const(*this).asBoolean());
    }

    const List<Value>& asList() const
    {
        if (!isList()) { throw std::runtime_error("Value is not a List"); }
        return std::get<List<Value>>(value);
    }

    List<Value>& asList()
    {
        return const_cast<List<Value>&>(std::as_const(*this).asList());
    }

    const Map<String, Value>& asMap() const
    {
        if (!isMap()) { throw std::runtime_error("Value is not a Map"); }
        return std::get<Map<String, Value>>(value);
    }

    Map<String, Value>& asMap()
    {
        return const_cast<Map<String, Value>&>(std::as_const(*this).asMap());
    }

    /// Gets the value at an attribute from a Map.
    ///
    /// Throws if called on a non-Map type or the attribute isn't set.
    ///
    /// A reference is returned so the interpreter can modify nested
    /// structures on the original Map.
    const Value& getAttribute(String attr) const
    {
        if (!isMap())
        {
            throw std::runtime_error("Only Maps have attributes");
        }
        return asMap().getAttribute(std::move(attr));
    }

    Value& getAttribute(String attr)
    {
        return const_cast<Value&>(std::as_const(*this).getAttribute(attr));
    }

    /// Sets or overwrites a named attribute on a Map.
    /// Throws if called on a non-Map type.
    void setAttribute(String attr, Value val)
    {
        if (!isMap())
        {
            throw std::runtime_error("Only Maps have attributes");
        }
        asMap().setAttribute(std::move(attr), std::move(val));
    }

    friend std::ostream& operator<<(std::ostream& os, const Value& v)
    {
        if (v.isString()) { os << v.asString(); }
        else if (v.isInteger()) { os << v.asInteger(); }
        else if (v.isBoolean()) { os << v.asBoolean(); }
        else if (v.isList()) { os << v.asList(); }
        else if (v.isMap()) { os << v.asMap(); }
        return os;
    }

    bool operator==(const Value& other) const noexcept
    {
        if (isString() && other.isString()) { return asString() == other.asString(); }
        else if (isInteger() && other.isInteger()) { return asInteger() == other.asInteger(); }
        else if (isBoolean() && other.isBoolean()) { return asBoolean() == other.asBoolean(); }
        else if (isList() && other.isList())  { return asList() == other.asList(); }
        else if (isMap() && other.isMap()) { return asMap() == other.asMap(); }
        return false;
    }
};

inline std::optional<bool> maybeCompareValues(const Value& lhs, const Value& rhs)
{
    if (lhs.isString() && rhs.isString())
    {
        return lhs.asString().value < rhs.asString().value;
    }
    if (lhs.isInteger() && rhs.isInteger())
    {
        return lhs.asInteger().value < rhs.asInteger().value;
    }
    if (lhs.isBoolean() && rhs.isBoolean())
    {
        return lhs.asBoolean().value < rhs.asBoolean().value;
    }
    return std::nullopt;
}

inline bool doLogicalOr(const Value& a, const Value& b)
{
    // For now, only booleans
    // TODO: Support truthy for more flexibility? Could look like:
    // return isTruthy(a) || isTruthy(b)
    return a.asBoolean().value || b.asBoolean().value;
}

inline bool doUnaryNot(const Value& a)
{
    // For now, only booleans
    // TODO: Support truthy for more flexibility? Could look like:
    // return !isTruthy(a);
    return !(a.asBoolean().value);
}

inline List<Value> sortList(const List<Value>& list, std::optional<String> key = {})
{
    List<Value> listCopy = list;

    std::sort(listCopy.value.begin(), listCopy.value.end(),
        [&key](const Value& lhs, const Value &rhs)
        {
            if (key.has_value())
            {
                // List elements are treated as maps, and values of the key are compared
                const Value& lhsValue = lhs.getAttribute(*key);
                const Value& rhsValue = rhs.getAttribute(*key);

                auto res = maybeCompareValues(lhsValue, rhsValue);
                if (res.has_value())
                {
                    return *res;
                }
            }
            else
            {
                // Otherwise, try to compare values directly
                auto res = maybeCompareValues(lhs, rhs);
                if (res.has_value())
                {
                    return *res;
                }
            }
            // If reached here, types couldn't be compared
            throw std::runtime_error(
                "List is not sortable because element types are not comparable"
            );
        }
    );

    return listCopy;
}
