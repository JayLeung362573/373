#pragma once

#include <string>
#include <variant>

#include "Types.h"

// Should these use our types? Might not make sense outside the interpreter layer

struct GetTextInputMessage
{
    String playerID;
    String prompt;
};

struct TextInputMessage
{
    String playerID;
    String prompt;
    String input;
};

struct GameMessage
{
    std::variant<GetTextInputMessage, TextInputMessage> inner;
};
