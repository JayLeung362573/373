#pragma once

#include <string>
#include <variant>
#include <vector>

#include "Types.h"

// Should these use our types? Might not make sense outside the interpreter layer
//--> let's use String

//Request Messages (Server->Player)
struct GetChoiceInputMessage
{
    String playerID;
    String prompt;
    List<Value> choices;
};

struct GetTextInputMessage
{
    String playerID;
    String prompt;
};

struct GetRangeInputMessage
{
    String playerID;
    String prompt;
    Integer minValue;
    Integer maxValue;
};

struct GetVoteInputMessage
{
    String playerID;
    String prompt;
    List<Value> choices;
};


//Response Messages (Player->Server)
struct ChoiceInputMessage
{
    String playerID;
    String prompt;
    String choice;
};

struct TextInputMessage
{
    String playerID;
    String prompt;
    String input;
};

struct RangeInputMessage
{
    String playerID;
    String prompt;
    Integer value;
};

struct VoteInputMessage
{
    String playerID;
    String prompt;
    String vote;
};



struct GameMessage
{
    std::variant<
        GetChoiceInputMessage,
        GetTextInputMessage,
        GetRangeInputMessage,
        GetVoteInputMessage,
        ChoiceInputMessage,
        TextInputMessage,
        RangeInputMessage,
        VoteInputMessage
        > inner;
};
