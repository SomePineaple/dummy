//
// Created by nj60 on 11/18/25.
//

#include "nn_player.h"

namespace rummy::nn {
    bool nn_player::run_turn(game_state *gs) {
        logic->init_gs(gs);
        if (const auto draw = logic->get_draw(); draw == 0) {
            draw_from_stock(gs, 1);
        } else {
            if (!draw_from_discard(gs, draw)) return false;
        }



        return true;
    }

    // TODO: Finish this.
    bool nn_player::try_play_cards(const vector<uint8_t>& cards) {
        meld m;
        for (const auto card : cards) {
            if (card >= hand_size()) return false;

            m.add_card(hand.get_card(card));
        }

        if (m.size() >= 3) {
            if (m.meld_type() != INVALID) {

            }
            return false;
        }
    }

    shared_ptr<clients::player> nn_player::clone() const {
        return make_shared<nn_player>(*this);
    }
} // rummy::nn