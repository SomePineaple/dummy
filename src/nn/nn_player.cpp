//
// Created by nj60 on 11/18/25.
//

#include "nn_player.h"

#include <random>

namespace rummy::nn {
    bool NNPlayer::run_turn(GameState *gs) {
        m_hand.sort();

        utils::LegalMoveEngine moveEngine(gs, m_hand);
        msp_logic->init_gs(gs);
        auto play_cards = msp_logic->get_play_cards(moveEngine.get_hand_play_mask());
        if (const auto draw = msp_logic->get_draw(moveEngine.get_discard_pile_mask()); draw == 0) {
            draw_from_stock(gs, 1);
            try_play_cards(play_cards, moveEngine);
        } else if (draw == gs->discardPile.size() - 1) {
            draw_from_discard(gs, 1);
            try_play_cards(play_cards, moveEngine);
        } else {
            play_cards = msp_logic->get_play_cards(moveEngine.get_hand_play_mask(gs->discardPile.get_card(draw - 1)->get_sort_value()));
            try_play_cards(play_cards, gs->discardPile.get_card(gs->discardPile.size() - draw - 1), moveEngine);
            if (!m_ToPlay.empty()) {
                if (!draw_from_discard(gs, draw)) return false;
                // Add the recently drawn card to the meld
                m_ToPlay.back().add_card(m_hand.get_card(get_hand_size() - 1));
                cout << "successfully drew from discard and played!" << endl;
            } else {
                draw_from_stock(gs, 1);
                try_play_cards(play_cards, moveEngine);
            }
        }

        const uint8_t discardIndex = msp_logic->get_discard(get_hand_size());
        if (discardIndex >= m_hand.size()) {
            return false;
        }

        // Save the card we want to discard, because the index will shift after we play cards.
        const auto toDiscard = m_hand.get_card(discardIndex);

        for (const auto& meld : m_ToPlay) {
            for (const auto& card : meld.get_cards()) {
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

    void NNPlayer::try_play_cards(const std::vector<uint8_t>& cards, utils::LegalMoveEngine& moveEngine) {
        std::vector cardsMut(cards);

        m_ToPlay.clear();

        for (auto& meld : moveEngine.get_playable_melds()) {
            sort(meld.begin(), meld.end(), std::less<>());
            if (includes(cardsMut.begin(), cardsMut.end(), meld.begin(), meld.end())) {
                Meld m;

                for (const auto& card : meld) {
                    m.add_card(m_hand.get_card(card));
                    if (const auto it = find(cardsMut.begin(), cardsMut.end(), card); it != cardsMut.end()) {
                        cardsMut.erase(it);
                    }
                }

                m_ToPlay.push_back(m);
            }
        }
    }

    void NNPlayer::try_play_cards(const std::vector<uint8_t> &cards, const shared_ptr<Card>& fromDiscard, utils::LegalMoveEngine& moveEngine) {
        m_ToPlay.clear();
        std::vector<std::vector<uint8_t>> possibleMelds = moveEngine.get_playable_melds(fromDiscard->get_sort_value());
        for (auto& m : possibleMelds) {
            sort(m.begin(), m.end(), std::less<>());
            if (includes(cards.begin(), cards.end(), m.begin(), m.end())) {
                Meld meld;
                for (const auto& card : m) {
                    meld.add_card(m_hand.get_card(card));
                }

                m_ToPlay.push_back(meld);
                return;
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
        uniform_int_distribution dist(0, m_hand.size() - 1);

        discard(gs, dist(rng));
    }

    uint16_t NNPlayer::get_unplayed_points() const {
        return m_hand.get_value();
    }

    shared_ptr<clients::Player> NNPlayer::clone() const {
        return make_shared<NNPlayer>(*this);
    }
} // rummy::nn
