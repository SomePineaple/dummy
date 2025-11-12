//
// Created by nj60 on 11/12/25.
//

#include "game.h"

namespace game {
    Game::Game() {
        p1 = new HumanPlayer("Player 1");
        p2 = new HumanPlayer("Player 2");

        gs = new GameState(p1, true);
    }

    Game::~Game() {
        delete p1;
        delete p2;
        delete gs;
    }

} // game