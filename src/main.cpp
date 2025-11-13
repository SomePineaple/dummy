#include <memory>
#include <string>

#include "game/game.h"
#include "game/humanplayer.h"
#include "signalbot/signalplayer.h"

int main(int numArgs, const char** args) {
    if (numArgs != 3)
        return 1;

    std::string playerNumber = args[1];
    std::string botNumber = args[2];

    auto consolePlayer = std::make_shared<game::clients::HumanPlayer>("Player 1");
    auto signalPlayer = std::make_shared<game::clients::SignalPlayer>(playerNumber, botNumber);
    game::GameState startingGs(consolePlayer, signalPlayer);

    consolePlayer->drawFromStock(&startingGs, 13);
    signalPlayer->drawFromStock(&startingGs, 13);

    auto g = game::Game{startingGs};
    while (!g.isGameOver())
        g.runRound();

    return 0;
}
