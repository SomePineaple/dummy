#include <memory>
#include <string>
#include <iostream>

#include "game/game.h"
#include "game/consoleplayer.h"
#include "signalbot/signalplayer.h"

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

    auto consolePlayer = std::make_shared<rummy::clients::ConsolePlayer>("Player 1");
    auto signalPlayer = std::make_shared<rummy::clients::SignalPlayer>(playerNumber, botNumber);

    auto g = rummy::Game{signalPlayer, consolePlayer};
    while (!g.isGameOver())
        g.runRound();

    return 0;
}
