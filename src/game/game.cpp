//
// Created by nj60 on 11/12/25.
//

#include "game.h"

namespace game {
    Game::Game() {
        p1 = make_shared<HumanPlayer>("Player 1");
        p2 = make_shared<HumanPlayer>("Player 2");

        gs = make_unique<GameState>(p1, true);
    }
} // game