#pragma once

#include <vector>
#include <span>
#include "GameInterpreter.h"
#include "Rules.h"

/**
 * Represents a running game. A call to `run()` will execute all rules of the game.
 *
 * Essentially a wrapper around GameInterpreter to encourage correct usage, since
 * it doesn't expose the `visit` methods.
 *
 * TODO: Support player IO once that's supported in GameInterpreter
 */
class GameRuntime {
    public:
        GameRuntime(ast::GameRules& rules) : m_interpreter(rules) {}

        // Run all rules of the game.
        // TODO: Doesn't support inputting or outputting game messages, because
        // input system is not fully developed yet in the interpreter.
        // In the future, the API may look something like this to support I/O:
        //      std::vector<GameMessage>
        //      tick(std::vector<GameMessage> inMessages)
        void run() {
            if (m_interpreter.isDone())
            {
                throw std::runtime_error("Calling run() on a finished game");
            }
            m_interpreter.run();
        }

        // getGameState (read-only)
        // Used mainly for tests to check if rules ran as expected
        const VariableMap& getGameState() const {
            return m_interpreter.getGameState();
        }

        bool
        isFinished() const {
            return m_interpreter.isDone();
        }

    private:
        GameInterpreter m_interpreter;
};
