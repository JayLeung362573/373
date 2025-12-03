#include <gtest/gtest.h>
#include "GameSession/GameSession.h"

using namespace ast;

// Create a trivial ruleset
static GameRules makeTrivialRules() {
    GameRules rules;
    rules.statements.push_back(
        makeInputText(
            makeVariable(Name{"player1"}),
            makeVariable(Name{"greeting"}),
            String{"Hello!"}
        )
    );
    return rules;
}

TEST(GameSessionTest, ConstructorRegistersPlayers) {
    std::vector<LobbyMember> players = {
        {1, "P1", LobbyRole::Player, true},
        {2, "P2", LobbyRole::Player, true}
    };

    auto rules = makeTrivialRules();
    GameSession session("lobby_test", std::move(rules), players);

    // Should not crash & interpreter variables should be set.
    ASSERT_FALSE(session.isFinished());
}

TEST(GameSessionTest, StartProducesInitialOutputAndRequests) {
    std::vector<LobbyMember> players = {
        {1, "player", LobbyRole::Player, true}
    };

    auto rules = makeTrivialRules();
    GameSession session("lobby_test", std::move(rules), players);

    auto out = session.start();

    // Should broadcast "Hello, world!"
    ASSERT_FALSE(out.empty());

    bool foundOutput = false;
    for (auto &msg : out) {
        if (msg.message.type == MessageType::GameOutput) {
            auto &g = std::get<GameOutputMessage>(msg.message.data);
            ASSERT_EQ(g.message, "Game starts!");
            foundOutput = true;
        }
    }

    ASSERT_TRUE(foundOutput);
}

TEST(GameSessionTest, StartTwiceDoesNotCrash) {
    std::vector<std::unique_ptr<Statement>> stmts;
    stmts.push_back(makeAssignment(
        makeVariable(Name{"winner"}),
        makeConstant(Value{String{"A"}})
    ));
    GameRules rules{std::move(stmts)};
    
    std::vector<LobbyMember> players = {
        {1, "player", LobbyRole::Player, true}
    };
    
    GameSession session("lobby_test", std::move(rules), players);
    
    EXPECT_NO_THROW(session.start());
    EXPECT_NO_THROW(session.start());  // second start should not throw
}

TEST(GameSessionTest, EmptyRulesAllowed) {
    EXPECT_NO_THROW(GameRules emptyRules{});
}

TEST(GameSessionTest, EmptyGameSessionEmptyRulesNotAllowed) {
    GameRules emptyRules{};

    std::vector<LobbyMember> players = {
        {1, "Player", LobbyRole::Player, true}
    };

    try {
        GameSession session("lobby_empty", std::move(emptyRules), players);
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& err) {
        EXPECT_STREQ(err.what(), "Can't create iterator: program must have at least one statement");
    }
    catch (...) {
        FAIL() << "Expected std::runtime_error but got different exception";
    }
}

TEST(GameSessionTest, EmptyGameSessionEmptyPlayersAllowed) {
    std::vector<LobbyMember> emptyPlayers = {};
    auto rules = makeTrivialRules();
    EXPECT_NO_THROW(GameSession session("lobby_empty", std::move(rules), emptyPlayers));
}

TEST(GameSessionTest, TickReturnsEmptyIfNoMessages) {
    std::vector<LobbyMember> players = {
        {1, "Player", LobbyRole::Player, true}
    };
    auto rules = makeTrivialRules();

    GameSession session("lobby_test", std::move(rules), players);

    auto startOut = session.start();
    ASSERT_FALSE(startOut.empty());
    EXPECT_TRUE(
            startOut[1].message.type == MessageType::RequestTextInput ||
            startOut[1].message.type == MessageType::RequestRangeInput
    );
    auto SessionOut = session.tick({});
    EXPECT_TRUE(SessionOut.empty());
}

TEST(GameSessionTest, TickGeneratesRequestsAfterInputRangeRule) {
    std::vector<std::unique_ptr<Statement>> stmts;
    stmts.push_back(
        makeInputRange(
            makeVariable(Name{"player1"}),
            makeVariable(Name{"val"}),
            String{"Enter"},
            makeConstant(Value{Integer{0}}),
            makeConstant(Value{Integer{10}})
        )
    );

    GameRules rules{std::move(stmts)};

    std::vector<LobbyMember> players = {
        {1, "player", LobbyRole::Player, true}
    };

    GameSession session("lobby_test", std::move(rules), players);

    // Requests are produced during start()
    auto startOut = session.start();

    bool foundRequest = false;
    for (auto &msg : startOut) {
        if (msg.message.type == MessageType::RequestRangeInput) {
            foundRequest = true;
        }
    }

    EXPECT_TRUE(foundRequest);

    // tick() should NOT produce the request again
    auto tickOut = session.tick({});
    EXPECT_TRUE(tickOut.empty());
}

TEST(GameSessionTest, TickRoutesInputAndContinuesExecution) {
    std::vector<LobbyMember> players = {
        {1, "player", LobbyRole::Player, true}
    };

    auto rules = makeTrivialRules();
    GameSession session("lobby_test", std::move(rules), players);
    session.start(); // produce initial outputs

    // Simulate a text input response
    ClientMessage incoming{
        1,
        { MessageType::ResponseTextInput, ResponseTextInputMessage{"Hi!", "Hello!"} }
    };

    auto out = session.tick({incoming});

    // Should eventually produce GameOver
    bool foundOver = false;
    for (auto &msg : out) {
        if (msg.message.type == MessageType::GameOver) {
            foundOver = true;
            break;
        }
    }

    ASSERT_TRUE(foundOver);
}
