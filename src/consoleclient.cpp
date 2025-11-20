//
// Created by nj60 on 11/17/25.
//

#include "game/game.h"
#include "game/consoleplayer.h"

using namespace rummy;

int main() {
    auto p1 = make_shared<clients::ConsolePlayer>("Player 1");
    auto p2 = make_shared<clients::ConsolePlayer>("Player 2");

    //GameState startingGs(p1, p2);

    Game game(p1, p2);
    while (game.is_game_over() == NOT_OVER)
        game.run_round();

    return 0;
}
