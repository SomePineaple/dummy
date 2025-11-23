//
// Created by nj60 on 11/18/25.
//

#ifndef DUMMY_NN_PLAYER_H
#define DUMMY_NN_PLAYER_H

#include <optional>

#include "nn_logic.h"
#include "../game/player.h"

namespace rummy::nn {
    class NNPlayer final : public clients::Player {
        shared_ptr<NNLogic> msp_logic;
        optional<Meld> mopt_ToPlay;
        void try_play_cards(const vector<uint8_t>& cards, const GameState* gs);
        void add_to_working_meld(const shared_ptr<Card>& card);
    public:
        NNPlayer(const network<sequential>& e, const network<sequential>& n) : msp_logic(make_shared<NNLogic>(e, n)), mopt_ToPlay(nullopt) {}

        bool run_turn(GameState *gs) override;
        [[nodiscard]] shared_ptr<Player> clone() const override;
    };
} // rummy::nn

#endif //DUMMY_NN_PLAYER_H
