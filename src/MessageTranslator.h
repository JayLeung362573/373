#pragma once
#include <string>
#include <variant>
#include "Message.h"

class MessageTranslator {
public:
    // Keep statics for stateless functionality without having to worry about runtime
    static std::string serialize(const Message& msg);
    static Message deserialize(const std::string& payload);
};