//
// Created by nj60 on 11/18/25.
//

#include "nn_player.h"

#include <random>

namespace rummy::nn {
    bool NNPlayer::run_turn(GameState *gs) {
        msp_logic->init_gs(gs);
        if (const auto draw = msp_logic->get_draw(); draw == 0) {
            draw_from_stock(gs, 1);
        } else {
            if (!draw_from_discard(gs, draw)) return false;
        }

        try_play_cards(msp_logic->get_play_cards(), gs);

        const uint8_t discardIndex = msp_logic->get_discard();
        if (discardIndex >= m_hand.size()) {
            return false;
        }

        // Save the card we want to discard, because the index will shift after we play cards.
        const auto toDiscard = m_hand.get_card(discardIndex);

        if (mopt_ToPlay != nullopt) {
            for (const auto& card : mopt_ToPlay->get_cards()) {
                add_to_working_meld(card);
            }

            play_working_meld(gs);
        }

        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i) == toDiscard)
                return discard(gs, i);
        }

        return false;
    }

    void NNPlayer::try_play_cards(const std::vector<uint8_t>& cards, const GameState* gs) {
        mopt_ToPlay = nullopt;
        Meld m;
        for (const auto card : cards) {
            if (card >= get_hand_size()) return;

            m.add_card(m_hand.get_card(card));
        }

        if (m.size() >= 3) {
            if (m.get_meld_type() != INVALID) {
                mopt_ToPlay = m;
            }
        } else {
            for (const auto& meld : gs->melds) {
                if (m.try_build_from(meld.get())) {
                    mopt_ToPlay = m;
                }
            }
        }
    }

    void NNPlayer::add_to_working_meld(const shared_ptr<Card>& card) {
        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i) == card) {
                m_WorkingMeld.add_card(card);
                m_hand.remove_at(i);
            }
        }
    }

    void NNPlayer::random_turn(GameState *gs) {
        draw_from_stock(gs, 1);

        // Discard a random card
        static std::mt19937 rng(std::random_device{}());
        uniform_int_distribution<int> dist(0, m_hand.size() - 1);

        discard(gs, dist(rng));
    }


    shared_ptr<clients::Player> NNPlayer::clone() const {
        return make_shared<NNPlayer>(*this);
    }
} // rummy::nn
