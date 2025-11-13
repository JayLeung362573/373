#include "Types.h"
#include <stdexcept>
#include <format>

// TODO: add a bit of documentation

class VariableMap
{
    public:
        void store(Name varName, const Value& value)
        {
            auto valuePtr = std::make_unique<Value>(value);
            m_map[varName] = std::move(valuePtr);
        }

        Value* load(Name varName)
        {
            if (!m_map.contains(varName))
            {
                throw std::runtime_error(
                    std::format("Variable with name '{}' doesn't exist in map", varName.name)
                );
            }
            return m_map[varName].get();
        }

        void del(Name varName)
        {
            m_map.erase(varName);
        }

    private:
        std::unordered_map<Name, std::unique_ptr<Value>> m_map;
};
