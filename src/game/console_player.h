//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_HUMANPLAYER_H
#define DUMMY_HUMANPLAYER_H

#include "player.h"

namespace rummy::clients {
    class console_player final : public player {
        std::string name;
        void print_game_state(const game_state* gs) const;
        bool ask_and_discard(game_state* gs);
        void ask_and_add(game_state* gs);
    public:
        explicit console_player(const std::string&  n) : name(n) {}

        bool run_turn(game_state* gs) override;
        [[nodiscard]] std::shared_ptr<player> clone() const override;
    };
} // game

#endif //DUMMY_HUMANPLAYER_H