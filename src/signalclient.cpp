#include <memory>
#include <string>
#include <iostream>

#include "game/game.h"
#include "game/console_player.h"
#include "signalbot/signal_player.h"

void printHelp() {
    std::cout << "This program requires a working installation of signal-cli in your path, and takes two arguments\n";
    std::cout << "1: <playerNumber> - the phone number of the person you want to play with\n";
    std::cout << "2: <botNUmber> - the phone number of the account registered with signal-cli" << std::endl;
}

using namespace std;
namespace rc = rummy::clients;
int main(const int numArgs, const char** args) {
    if (numArgs != 3) {
        printHelp();
        return 1;
    }

    string playerNumber = args[1];
    string botNumber = args[2];

    auto game = rummy::Game{make_shared<rc::SignalPlayer>(playerNumber, botNumber), make_shared<rc::ConsolePlayer>("Player 1")};
    while (game.is_game_over() == rummy::NOT_OVER)
        game.run_round();

    game.notify_players();

    return 0;
}
