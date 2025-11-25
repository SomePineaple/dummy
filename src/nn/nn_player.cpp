//
// Created by nj60 on 11/18/25.
//

#include "nn_player.h"

#include <random>

namespace rummy::nn {
    bool NNPlayer::run_turn(GameState *gs) {
        m_PlayableMelds.clear();
        m_PlayableMeldsWithDiscard.clear();
        std::vector<bool> playMask = get_hand_play_mask(gs);
        const std::vector<bool> discardMask = get_discard_pile_mask(gs);
        msp_logic->init_gs(gs);
        auto play_cards = msp_logic->get_play_cards(playMask);
        if (const auto draw = msp_logic->get_draw(discardMask); draw == 0) {
            draw_from_stock(gs, 1);
            try_play_cards(play_cards);
        } else if (draw == 1) {
            draw_from_discard(gs, 1);
            try_play_cards(play_cards);
        } else {
            playMask = get<1>(m_PlayableMeldsWithDiscard[gs->discardPile.get_card(draw - 1)->get_sort_value()]);
            play_cards = msp_logic->get_play_cards(playMask);
            try_play_cards(play_cards, gs->discardPile.get_card(draw - 1));
            if (!m_ToPlay.empty()) {
                if (!draw_from_discard(gs, draw)) return false;
                // Add the recently drawn card to the meld
                m_ToPlay.back().add_card(m_hand.get_card(get_hand_size() - 1));
                cout << "successfully drew from discard and played!" << endl;
            } else {
                draw_from_stock(gs, 1);
                try_play_cards(play_cards);
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

    bool NNPlayer::can_draw_discard(const shared_ptr<Card> &card) {
        bool canDrawDiscard = false;
        std::vector<bool> mask(m_hand.size());

        std::array<std::vector<uint8_t>, 4> suits;
        std::array<std::vector<uint8_t>, 13> ranks;

        for (int i = 0; i < get_hand_size(); i++) {
            const auto c = m_hand.get_card(i);
            suits[c->suit].push_back(i);
            ranks[c->value - 1].push_back(i);
        }

        std::vector<std::vector<uint8_t>> playableMelds;

        if (ranks[card->value - 1].size() >= 2) {
            playableMelds.push_back(ranks[card->value - 1]);
            canDrawDiscard = true;
            for (const auto& c : ranks[card->value - 1])
                mask[c] = true;
        }

        uint8_t up1 = 100, up2 = 100, down1 = 100, down2 = 100;
        for (const auto& c : suits[card->suit]) {
            if (const auto val = m_hand.get_card(c)->value; val == card->value + 1) {
                up1 = c;
            } else if (val == card->value -1) {
                down1 = c;
            } else if (val == card->value + 2) {
                up2 = c;
            } else if (val == card->value - 2) {
                down2 = c;
            }
        }

        if (up1 != 100 && up2 != 100) {
            std::vector<uint8_t> runVector;
            runVector.push_back(up1);
            runVector.push_back(up2);
            playableMelds.push_back(runVector);
            mask[up1] = true;
            mask[up2] = true;
            canDrawDiscard = true;
        }

        if (down1 != 100 && down2 != 100) {
            std::vector<uint8_t> runVector;
            runVector.push_back(down2);
            runVector.push_back(down1);
            playableMelds.push_back(runVector);
            mask[down1] = true;
            mask[down2] = true;
            canDrawDiscard = true;
        }

        if (up1 != 100 && down1 != 100) {
            std::vector<uint8_t> runVector;
            runVector.push_back(down1);
            runVector.push_back(up1);
            playableMelds.push_back(runVector);
            mask[down1] = true;
            mask[up1] = true;
            canDrawDiscard = true;
        }

        m_PlayableMeldsWithDiscard[card->get_sort_value()] = tuple(playableMelds, mask);

        return canDrawDiscard;
    }

    std::vector<bool> NNPlayer::get_discard_pile_mask(const GameState *gs) {
        std::vector<bool> mask(gs->discardPile.size());

        for (int i = 0; i < gs->discardPile.size(); i++) {
            if (can_draw_discard(gs->discardPile.get_card(i)))
                mask[i] = true;
        }

        return mask;
    }

    std::vector<bool> NNPlayer::get_hand_play_mask(const GameState* gs) {
        std::vector<bool> mask(get_hand_size());

        std::array<std::vector<uint8_t>, 4> suits;
        std::array<std::vector<uint8_t>, 13> ranks;

        for (int i = 0; i < get_hand_size(); i++) {
            const auto card = m_hand.get_card(i);
            suits[card->suit].push_back(i);
            ranks[card->value - 1].push_back(i);
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
                const auto& cardsOfSameRank = ranks[meld->get_card(0)->value - 1];
                if (!cardsOfSameRank.empty()) {
                    m_PlayableMelds.push_back(cardsOfSameRank);
                    mask[cardsOfSameRank[0]];
                }
            }
        }

        return mask;
    }

    void NNPlayer::try_play_cards(const std::vector<uint8_t>& cards) {
        std::vector cardsMut(cards);

        m_ToPlay.clear();

        for (auto& meld : m_PlayableMelds) {
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

    void NNPlayer::try_play_cards(const std::vector<uint8_t> &cards, const shared_ptr<Card>& fromDiscard) {
        m_ToPlay.clear();
        std::vector<std::vector<uint8_t>> possibleMelds = get<0>(m_PlayableMeldsWithDiscard[fromDiscard->get_sort_value()]);
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
        return m_hand.calc_points();
    }


    shared_ptr<clients::Player> NNPlayer::clone() const {
        return make_shared<NNPlayer>(*this);
    }
} // rummy::nn
