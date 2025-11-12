#include "game/game.h"

int main() {
    auto game = game::Game{};

    while (!game.isGameOver())
        game.runRound();

    return 0;
}
