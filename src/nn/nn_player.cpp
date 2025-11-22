//
// Created by nj60 on 11/18/25.
//

#include "nn_player.h"

namespace rummy::nn {
    bool nn_player::run_turn(GameState *gs) {
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
        Meld m;
        for (const auto card : cards) {
            if (card >= get_hand_size()) return false;

            m.add_card(m_hand.get_card(card));
        }

        if (m.size() >= 3) {
            if (m.get_meld_type() != INVALID) {

            }
            return false;
        }

        return true;
    }

    shared_ptr<clients::Player> nn_player::clone() const {
        return make_shared<nn_player>(*this);
    }
} // rummy::nn