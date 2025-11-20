#include <memory>
#include <string>
#include <iostream>

#include "game/game.h"
#include "game/console_player.h"
#include "signalbot/signal_player.h"

void printHelp() {
    std::cout << "This program requires a working installation of signal-cli in your path, and takes two arguments\n";
    std::cout << "1: <playerNumber> - the phone number of the person you want to play with\n";
    std::cout << "2: <botNUmber> - the phone number of the account registered with signal-cli" << endl;
}

int main(const int numArgs, const char** args) {
    if (numArgs != 3) {
        printHelp();
        return 1;
    }

    std::string playerNumber = args[1];
    std::string botNumber = args[2];

    auto g = rummy::game{std::make_shared<rummy::clients::console_player>("Player 1"), std::make_shared<rummy::clients::signal_player>(playerNumber, botNumber)};
    while (g.get_winner() == rummy::NOT_OVER)
        g.run_round();

    return 0;
}
