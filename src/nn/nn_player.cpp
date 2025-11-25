//
// Created by nj60 on 11/18/25.
//

#include "nn_player.h"

#include <random>

namespace rummy::nn {
    bool NNPlayer::run_turn(GameState *gs) {
        m_PlayableMelds.clear();
        const std::vector<bool> playMask = get_hand_play_mask(gs);
        msp_logic->init_gs(gs);
        const auto play_cards = msp_logic->get_play_cards(playMask);
        if (const auto draw = msp_logic->get_draw(gs->discardPile.size()); draw == 0) {
            draw_from_stock(gs, 1);
            try_play_cards(play_cards, gs);
        } else {
            try_play_cards(play_cards, gs->discardPile.get_card(draw - 1), gs);
            if (mopt_ToPlay != nullopt) {
                if (!draw_from_discard(gs, draw)) return false;
            } else {
                draw_from_stock(gs, 1);
                try_play_cards(play_cards, gs);
            }
        }

        const uint8_t discardIndex = msp_logic->get_discard(get_hand_size());
        if (discardIndex >= m_hand.size()) {
            return false;
        }

        // Save the card we want to discard, because the index will shift after we play cards.
        const auto toDiscard = m_hand.get_card(discardIndex);

        if (mopt_ToPlay != nullopt) {
            for (const auto& card : mopt_ToPlay->get_cards()) {
                add_to_working_meld(card);
            }

            cout << "Played a meld" << endl;
            play_working_meld(gs);
        }

        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i) == toDiscard)
                return discard(gs, i);
        }

        return false;
    }

    std::vector<bool> NNPlayer::get_hand_play_mask(const GameState* gs) {
        std::vector<bool> mask(get_hand_size());

        std::array<std::vector<uint8_t>, 4> suits;
        std::array<std::vector<uint8_t>, 13> ranks;

        for (int i = 0; i < get_hand_size(); i++) {
            const auto card = m_hand.get_card(i);
            suits[card->suit].push_back(i);
            ranks[card->value].push_back(i);
        }

        // Check for complete valid sets
        for (const auto& rank : ranks) {
            if (rank.size() >= 3) {
                for_each(rank.begin(), rank.end(), [&mask](const auto& card) {mask[card] = true;});
                m_PlayableMelds.push_back(rank);
            }
        }

        // Check for all complete valid runs.
        for (auto& suit : suits) {
            sort(suit.begin(), suit.end(), [this](const auto& a, const auto& b) {
                return m_hand.get_card(a)->value < m_hand.get_card(b)->value;
            });
            if (suit.size() >= 3) {
                // Check for high ace
                if (m_hand.get_card(suit[0])->value == 1 && !(m_hand.get_card(suit[1])->value == 2 && m_hand.get_card(suit[2])->value == 3)) {
                    suit.push_back(suit[0]);
                    suit.erase(suit.begin());
                }

                std::vector<uint8_t> buildingMeld;
                buildingMeld.push_back(suit[0]);
                for (int i = 1; i < suit.size(); i++) {
                    if (m_hand.get_card(i)->value == m_hand.get_card(i - 1)->value + 1) {
                        buildingMeld.push_back(suit[i]);
                        if (buildingMeld.size() >= 3) {
                            mask[suit[i]] = true;
                            mask[suit[i - 1]] = true;
                            mask[suit[i - 2]] = true;
                            m_PlayableMelds.push_back(buildingMeld);
                        }
                    } else {
                        buildingMeld.clear();
                        buildingMeld.push_back(suit[i]);
                    }
                }
            }
        }

        for (const auto& meld : gs->melds) {
            if (const auto mType = meld->get_meld_type(); mType == RUN) {
                auto meldCards = meld->get_cards();
                sort(meldCards.begin(), meldCards.end(), [](const auto& a, const auto& b) {
                   return a->value < b->value;
                });
                // Again check for high aces.
                if (meldCards[0]->value == 1 && meldCards[1]->value != 2) {
                    meldCards.push_back(meldCards[0]);
                    meldCards.erase(meldCards.begin());
                }

                const auto startValue = meldCards[0]->value;
                auto endValue = meldCards.back()->value;
                // Cant build off high aces
                if (endValue == 1) endValue = 100;

                for (const auto& card : suits[meldCards[0]->suit]) {
                    const auto value = m_hand.get_card(card)->value;
                    if (value == startValue - 1 || value == endValue + 1) {
                        mask[card] = true;
                        std::vector<uint8_t> v;
                        v.push_back(card);
                        m_PlayableMelds.push_back(v);
                    }
                }
            } else if (mType == SET) {
                const auto& cardsOfSameRank = ranks[meld->get_card(0)->value];
                if (!cardsOfSameRank.empty()) {
                    m_PlayableMelds.push_back(cardsOfSameRank);
                    mask[cardsOfSameRank[0]];
                }
            }
        }

        return mask;
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

    void NNPlayer::try_play_cards(const std::vector<uint8_t> &cards, const shared_ptr<Card>& fromDiscard, const GameState *gs) {
        mopt_ToPlay = nullopt;
        Meld m;
        for (const auto card : cards) {
            if (card >= get_hand_size()) return;

            m.add_card(m_hand.get_card(card));
        }

        m.add_card(fromDiscard);

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
        uniform_int_distribution dist(0, m_hand.size() - 1);

        discard(gs, dist(rng));
    }

    shared_ptr<clients::Player> NNPlayer::clone() const {
        return make_shared<NNPlayer>(*this);
    }
} // rummy::nn
