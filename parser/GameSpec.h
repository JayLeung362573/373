#pragma once

#include <string>
#include <vector>
#include <optional>

// This is currently based upon the games/rock-paper-scissors.game
//this can be expanded in the future once we have more bits
struct PlayerRange {
    int min = 0;
    int max = 0;
};


struct SetupRule { // from the config
    /*
      setup: {
        rounds {
              kind: integer
              prompt: "The number of rounds to play"
              range: (1, 20)
        }
      }
     */
    std::string id; // rounds
    std::string kind; // integer
    std::string prompt;
    std::optional<PlayerRange> range;
};
struct GameSpec {
    std::string name;
    PlayerRange playerRange;
    bool hasAudience = false;
    std::vector<SetupRule> setup;

    std::string constants;
    std::string variables;
};