#ifndef MAIN_GAMESPECLOADER_H
#define MAIN_GAMESPECLOADER_H
#include "GameSpec.h"
#include "TreeSitterUtil.h"

class GameSpecLoader {
public:
    GameSpec loadFile(const char* path);
    GameSpec loadString(const std::string& text);
    //the loadString is for testing purposes.
    /* - Run this (in main or wherever) to test an example game.
     std::string text = R"(
        configuration {
          name: "Test Game"
          player range: (2, 4)
          audience: false
          setup: {
            rounds {
              kind: integer
              prompt: "Rounds?"
              range: (1, 20)
            }
          }
        })";
     */
    bool HelloWorldSmokeTest(const char* path);
private:
    static void parseConfiguration(const std::string& src, TSNode node, GameSpec& spec);
    static void parsePlayerRange(const std::string& src, TSNode node, GameSpec& spec);
    static void parseSetup(const std::string& src, TSNode node, GameSpec& spec);
    static void parseRules(const std::string &src, TSNode node, GameSpec &spec);
};


#endif //MAIN_GAMESPECLOADER_H
