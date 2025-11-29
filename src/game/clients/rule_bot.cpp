//
// Created by nj60 on 11/26/25.
//

#include "rule_bot.h"

#include <random>

namespace rummy::clients {
    static std::mt19937 rng(std::random_device{}());

    bool RuleBot::run_turn(GameState *gs) {
        utils::LegalMoveEngine moveEngine(gs, m_hand);
        std::uniform_real_distribution<float> chance(0, 1);

        uint8_t drawn = 0;
        // 70% of the time we draw from stock
        if (chance(rng) < 0.7) {
            draw_from_stock(gs, 1);
            cout << "Drawing from stock" << endl;
        } else {
            auto discardMask = moveEngine.get_discard_pile_mask();
            for (int i = discardMask.size() - 1; i >= 0; i--) {
                // 50% chance of each drawable discard. Makes it more likely to draw earlier ones and not take too many cards
                if (discardMask[i] && chance(rng) < 0.5) {
                    cout << "We are drawing the " << gs->discardPile.get_card(i)->to_string() << " from discard" << endl;
                    draw_from_discard(gs, discardMask.size() - i);
                    drawn = m_WorkingMeld.get_card(0)->get_sort_value();
                    break;
                }
            }
        }

        if (drawn) {
            const auto playableMelds = moveEngine.get_playable_melds(drawn);
            try_play_cards(playableMelds[0], m_WorkingMeld.get_card(0), moveEngine);
        } else {
            const auto playableMelds = moveEngine.get_playable_melds();
            for (const auto& playableMeld : playableMelds) {
                if (chance(rng) < 0.5) {
                    try_play_cards(playableMeld);
                }
            }
        }

        m_hand.sort();
        for (auto& meld : m_ToPlay) {
            meld.sort();
            auto handCards = m_hand.get_cards();
            m_WorkingMeld = Meld{};
            if (auto meldCards = meld.get_cards(); includes(handCards.begin(), handCards.end(), meldCards.begin(), meldCards.end())) {
                for (const auto& card : meld.get_cards()) {
                    add_to_working_meld(card);
                }

                play_working_meld(gs);
                cout << "We are playing something" << endl;
            }
        }

        if (m_hand.size() > 0) {
            std::uniform_int_distribution<uint8_t> toDiscard(0, m_hand.size() - 1);
            const uint8_t d = toDiscard(rng);
            cout << "We are discarding " << m_hand.get_card(d)->to_string() << endl;
            discard(gs, d);
        }

        return true;
    }

    void RuleBot::try_play_cards(const std::vector<uint8_t>& cards) {
        Meld m;

        for (const auto& card : cards) {
            m.add_card(m_hand.get_card(card));
        }

        m_ToPlay.push_back(m);
    }

    void RuleBot::try_play_cards(const std::vector<uint8_t>& cards, const shared_ptr<Card>& fromDiscard, utils::LegalMoveEngine& moveEngine) {
        Meld meld;
        meld.add_card(fromDiscard);
        for (const auto& card : cards) {
            meld.add_card(m_hand.get_card(card));
        }

        m_ToPlay.push_back(meld);
    }

    void RuleBot::add_to_working_meld(const shared_ptr<Card>& card) {
        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i) == card) {
                m_WorkingMeld.add_card(card);
                m_hand.remove_at(i);
            }
        }
    }

    std::shared_ptr<Player> RuleBot::clone() const {
        return std::make_shared<RuleBot>(*this);
    }
} // rummy::clients
