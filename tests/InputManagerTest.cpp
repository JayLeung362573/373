#include <gtest/gtest.h>
#include "InputManager.h"

class InputManagerTest : public ::testing::Test {
protected:
    InputManager inputManager;
};

TEST_F(InputManagerTest, TextInput_NoResponse) {
    auto response = inputManager.getTextInput(String{"p1"}, String{"Name?"});
    EXPECT_FALSE(response.has_value());
}

TEST_F(InputManagerTest, TextInput_CreatesRequest) {
    inputManager.getTextInput(String{"p1"}, String{"Name?"});
    auto messages = inputManager.getPendingRequests();
    ASSERT_EQ(messages.size(), 1);
    auto* getInput = std::get_if<GetTextInputMessage>(&messages[0].inner);
    ASSERT_NE(getInput, nullptr);
    EXPECT_EQ(getInput->playerID, String{"p1"});
    EXPECT_EQ(getInput->prompt, String{"Name?"});
}

TEST_F(InputManagerTest, TextInput_ReturnsAfterResponse) {
    inputManager.getTextInput(String{"p1"}, String{"Name?"});
    std::vector<GameMessage> responses = {
        GameMessage{TextInputMessage{String{"p1"}, String{"Name?"}, String{"Alice"}}}
    };
    inputManager.handleIncomingMessages(responses);
    auto response = inputManager.getTextInput(String{"p1"}, String{"Name?"});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(*response, String{"Alice"});
}

TEST_F(InputManagerTest, PreventsDuplicate) {
    inputManager.getTextInput(String{"p1"}, String{"Name?"});
    inputManager.getTextInput(String{"p1"}, String{"Name?"});
    auto messages = inputManager.getPendingRequests();
    EXPECT_EQ(messages.size(), 1);
}

TEST_F(InputManagerTest, AllowsDifferentPrompts) {
    inputManager.getTextInput(String{"p1"}, String{"Name?"});
    inputManager.getTextInput(String{"p1"}, String{"Age?"}); //different prompt
    auto messages = inputManager.getPendingRequests();
    EXPECT_EQ(messages.size(), 2);
}

TEST_F(InputManagerTest, AllowsDifferentPlayers) {
    inputManager.getTextInput(String{"p1"}, String{"Name?"});
    inputManager.getTextInput(String{"p2"}, String{"Name?"}); //different player
    auto messages = inputManager.getPendingRequests();
    EXPECT_EQ(messages.size(), 2);
}

TEST_F(InputManagerTest, MultiplePlayers) {
    inputManager.getTextInput(String{"p1"}, String{"Q1"});
    inputManager.getTextInput(String{"p2"}, String{"Q1"});
    inputManager.getTextInput(String{"p3"}, String{"Q1"});

    std::vector<GameMessage> responses = {
        GameMessage{TextInputMessage{String{"p1"}, String{"Q1"}, String{"A1"}}},
        GameMessage{TextInputMessage{String{"p2"}, String{"Q1"}, String{"A2"}}},
        GameMessage{TextInputMessage{String{"p3"}, String{"Q1"}, String{"A3"}}}
    };
    inputManager.handleIncomingMessages(responses);

    EXPECT_EQ(inputManager.getTextInput(String{"p1"}, String{"Q1"}), String{"A1"});
    EXPECT_EQ(inputManager.getTextInput(String{"p2"}, String{"Q1"}), String{"A2"});
    EXPECT_EQ(inputManager.getTextInput(String{"p3"}, String{"Q1"}), String{"A3"});
}

TEST_F(InputManagerTest, ChoiceRequest) {
    List<Value> choices;
    choices.value.push_back(Value{String{"Rock"}});
    choices.value.push_back(Value{String{"Paper"}});

    inputManager.getChoiceInput(String{"p1"}, String{"Choose"}, choices);

    auto messages = inputManager.getPendingRequests();
    auto* getChoice = std::get_if<GetChoiceInputMessage>(&messages[0].inner);
    ASSERT_NE(getChoice, nullptr);
    EXPECT_EQ(getChoice->choices.value.size(), 2);
}

TEST_F(InputManagerTest, ChoiceResponse) {
    List<Value> choices;
    choices.value.push_back(Value{String{"Rock"}});
    inputManager.getChoiceInput(String{"p1"}, String{"Choose"}, choices);

    std::vector<GameMessage> responses = {
        GameMessage{ChoiceInputMessage{String{"p1"}, String{"Choose"}, String{"Rock"}}}
    };
    inputManager.handleIncomingMessages(responses);

    auto response = inputManager.getChoiceInput(String{"p1"}, String{"Choose"}, choices);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(*response, String{"Rock"});
}

TEST_F(InputManagerTest, RangeValid) {
    inputManager.getRangeInput(String{"p1"}, String{"Pick"}, Integer{1}, Integer{10});

    std::vector<GameMessage> responses = {
        GameMessage{RangeInputMessage{String{"p1"}, String{"Pick"}, 5}}
    };
    inputManager.handleIncomingMessages(responses);

    auto response = inputManager.getRangeInput(String{"p1"}, String{"Pick"}, Integer{1}, Integer{10});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(*response, Integer{5});
}

TEST_F(InputManagerTest, RangeOnOutOfRange) {
    std::vector<GameMessage> responses = {
        GameMessage{RangeInputMessage{String{"p1"}, String{"Pick"}, Integer{15}}}  //out of range
    };
    inputManager.handleIncomingMessages(responses);

    EXPECT_THROW({
        inputManager.getRangeInput(String{"p1"}, String{"Pick"}, Integer{1}, Integer{10});
    }, std::runtime_error);
}

TEST_F(InputManagerTest, ClearPendingRequests) {
    inputManager.getTextInput(String{"p1"}, String{"Q1"});
    inputManager.getTextInput(String{"p2"}, String{"Q2"});

    auto messages1 = inputManager.getPendingRequests();
    EXPECT_EQ(messages1.size(), 2);

    inputManager.clearPendingRequests();
    auto messages2 = inputManager.getPendingRequests();
    EXPECT_EQ(messages2.size(), 0);
}

TEST_F(InputManagerTest, ResponsesPersistAfterClear) {
    inputManager.getTextInput(String{"p1"}, String{"Name?"});

    std::vector<GameMessage> responses = {
        GameMessage{TextInputMessage{String{"p1"}, String{"Name?"}, String{"Alice"}}}
    };
    inputManager.handleIncomingMessages(responses);

    inputManager.clearPendingRequests();
    auto response = inputManager.getTextInput(String{"p1"}, String{"Name?"});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(*response, String{"Alice"});
}

TEST_F(InputManagerTest, EmptyMessageList) {
    std::vector<GameMessage> empty;
    EXPECT_NO_THROW(inputManager.handleIncomingMessages(empty));
}


TEST_F(InputManagerTest, UnmatchedResponse) {
    std::vector<GameMessage> responses = {
        GameMessage{TextInputMessage{String{"p1"}, String{"Random?"}, String{"Value"}}}
    };

    EXPECT_NO_THROW(inputManager.handleIncomingMessages(responses));

    auto response = inputManager.getTextInput(String{"p1"}, String{"Random?"});
    EXPECT_TRUE(response.has_value());
}
//testing when response arrives before any request, it should store safely w/o exception
//later request should retrieve the saved response





