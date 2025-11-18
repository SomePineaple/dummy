//
// Created by nj60 on 11/18/25.
//

#ifndef DUMMY_NN_PLAYER_H
#define DUMMY_NN_PLAYER_H

#include "nn_logic.h"
#include "../game/player.h"

namespace rummy::nn {
    class nn_player final : public clients::player {
        nn_logic logic;
    public:
        nn_player(const network<sequential>& e, const network<sequential>& n) : logic{e, n} {}
    }; // nn
} // rummy

#endif //DUMMY_NN_PLAYER_H