//
// Created by nj60 on 11/17/25.
//

#include "game/game.h"
#include "game/clients/console_player.h"
#include "game/clients/rule_bot.h"

using namespace rummy;

int main() {
    Game game{std::make_shared<clients::ConsolePlayer>("Player 1"), std::make_shared<clients::RuleBot>(true)};
    while (game.is_game_over() == GameStatus::NOT_OVER)
        game.run_round();

    game.notify_players();

    return 0;
}
