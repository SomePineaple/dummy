//
// Created by nj60 on 11/17/25.
//

#include "game/game.h"
#include "game/clients/console_player.h"

using namespace rummy;

int main() {
    Game game{make_shared<clients::ConsolePlayer>("Player 1"), make_shared<clients::ConsolePlayer>("Player 2")};
    while (game.is_game_over() == NOT_OVER)
        game.run_round();

    game.notify_players();

    return 0;
}
