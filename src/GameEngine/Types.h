// Defines types used by the interpreter to store game state.

#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <stdexcept>

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
    V& getAttribute(K attr)
    {
        if (!value.contains(attr))
        {
            throw std::runtime_error("Attribute does not exist in Map");
        }
        return value[attr];
    }

    /// Sets or overwrites a named attribute.
    void setAttribute(K attr, V val)
    {;
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
    std::variant<List<Value>, Map<String, Value>, String> value;

    bool isString() const
    {
        return std::holds_alternative<String>(value);
    }

    bool isList() const
    {
        return std::holds_alternative<List<Value>>(value);
    }

    bool isMap() const
    {
        return std::holds_alternative<Map<String, Value>>(value);
    }

    String& asString()
    {
        if (!isString()) { throw std::runtime_error("Value is not a String"); }
        return std::get<String>(value);
    }

    const String& asString() const
    {
        return const_cast<Value*>(this)->asString();
    }

    List<Value>& asList()
    {
        if (!isList()) { throw std::runtime_error("Value is not a List"); }
        return std::get<List<Value>>(value);
    }

    const List<Value>& asList() const
    {
        return const_cast<Value*>(this)->asList();
    }

    Map<String, Value>& asMap()
    {
        if (!isMap()) { throw std::runtime_error("Value is not a Map"); }
        return std::get<Map<String, Value>>(value);
    }

    const Map<String, Value>& asMap() const
    {
        return const_cast<Value*>(this)->asMap();
    }

    /// Gets the value at an attribute from a Map.
    ///
    /// Throws if called on a non-Map type or the attribute isn't set.
    ///
    /// A reference is returned so the interpreter can modify nested
    /// structures on the original Map.
    Value& getAttribute(String attr)
    {
        if (!isMap())
        {
            throw std::runtime_error("Only Maps have attributes");
        }
        return asMap().getAttribute(std::move(attr));
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
        else if (v.isList()) { os << v.asList(); }
        else if (v.isMap()) { os << v.asMap(); }
        return os;
    }

    bool operator==(const Value& other) const noexcept
    {
        return value == other.value;
    }
};
