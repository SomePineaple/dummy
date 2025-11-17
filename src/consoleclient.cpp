//
// Created by nj60 on 11/17/25.
//

#include "game/game.h"
#include "game/console_player.h"

using namespace rummy;

int main() {
    auto p1 = make_shared<clients::console_player>("Player 1");
    auto p2 = make_shared<clients::console_player>("Player 2");

    //GameState startingGs(p1, p2);

    game game(p1, p2);
    while (game.get_winner() != NOT_OVER)
        game.run_round();

    return 0;
}
