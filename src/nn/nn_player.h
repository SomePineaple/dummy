//
// Created by nj60 on 11/18/25.
//

#ifndef DUMMY_NN_PLAYER_H
#define DUMMY_NN_PLAYER_H

#include "nn_logic.h"
#include "../game/player.h"

namespace rummy::nn {
    class nn_player final : public clients::player {
        shared_ptr<nn_logic> logic;
        bool try_play_cards(const vector<uint8_t>& cards);
    public:
        nn_player(const network<sequential>& e, const network<sequential>& n) : logic(make_shared<nn_logic>(e, n)) {}

        bool run_turn(game_state *gs) override;
        [[nodiscard]] shared_ptr<player> clone() const override;
    };
} // rummy::nn

#endif //DUMMY_NN_PLAYER_H
