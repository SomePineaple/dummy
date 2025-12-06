//
// Created by nj60 on 11/26/25.
//

#include "legal_move_engine.h"

namespace rummy::utils {
    LegalMoveEngine::LegalMoveEngine(const GameState& gs, const Pile& hand) {
        update_no_draw(gs, hand);
        update_discard(gs, hand);
    }

    void LegalMoveEngine::update_no_draw(const GameState& gs, const Pile& hand) {
        std::vector<bool> mask(hand.size());
        std::vector<std::vector<uint8_t>> playableMelds;

        std::array<std::vector<uint8_t>, 4> suits;
        std::array<std::vector<uint8_t>, 13> ranks;

        for (int i = 0; i < hand.size(); i++) {
            const auto card = hand.get_card(i);
            suits[static_cast<size_t>(card.suit)].push_back(i);
            ranks[card.value - 1].push_back(i);
        }

        // Check for complete valid sets
        for (const auto& rank : ranks) {
            if (rank.size() >= 3) {
                for_each(rank.begin(), rank.end(), [&mask](const auto& card) {mask[card] = true;});
                playableMelds.push_back(rank);
            }
        }

        // Check for all complete valid runs.
        for (auto& suit : suits) {
            sort(suit.begin(), suit.end(), [&hand](const auto& a, const auto& b) {
                return hand.get_card(a).value < hand.get_card(b).value;
            });
            if (suit.size() >= 3) {
                // Check for high ace
                if (hand.get_card(suit[0]).value == 1 && !(hand.get_card(suit[1]).value == 2 && hand.get_card(suit[2]).value == 3)) {
                    suit.push_back(suit[0]);
                    suit.erase(suit.begin());
                }

                std::vector<uint8_t> buildingMeld;
                buildingMeld.push_back(suit[0]);
                for (int i = 1; i < suit.size(); i++) {
                    if (hand.get_card(i).value == hand.get_card(i - 1).value + 1) {
                        if (buildingMeld.empty()) {
                            buildingMeld.push_back(suit[i - 1]);
                        }
                        buildingMeld.push_back(suit[i]);
                        if (buildingMeld.size() >= 3) {
                            mask[suit[i]] = true;
                            mask[suit[i - 1]] = true;
                            mask[suit[i - 2]] = true;
                            playableMelds.push_back(buildingMeld);
                        }
                    } else {
                        buildingMeld.clear();
                        buildingMeld.push_back(suit[i]);
                    }
                }
            }
        }

        for (const auto& meld : gs.melds) {
            if (const auto mType = meld->get_meld_type(); mType == RUN) {
                auto meldCards = meld->get_cards();
                sort(meldCards.begin(), meldCards.end(), [](const auto& a, const auto& b) {
                   return a.value < b.value;
                });
                // Again check for high aces.
                if (meldCards[0].value == 1 && meldCards[1].value != 2) {
                    meldCards.push_back(meldCards[0]);
                    meldCards.erase(meldCards.begin());
                }

                const auto startValue = meldCards[0].value;
                auto endValue = meldCards.back().value;
                // Cant build off high aces
                if (endValue == 1) endValue = 100;

                for (const auto& card : suits[static_cast<size_t>(meldCards[0].suit)]) {
                    const auto value = hand.get_card(card).value;
                    if (value == startValue - 1 || value == endValue + 1) {
                        mask[card] = true;
                        std::vector<uint8_t> v;
                        v.push_back(card);
                        playableMelds.push_back(v);
                    }
                }
            } else if (mType == SET) {
                const auto& cardsOfSameRank = ranks[meld->get_card(0).value - 1];
                if (!cardsOfSameRank.empty()) {
                    playableMelds.push_back(cardsOfSameRank);
                    mask[cardsOfSameRank[0]] = true;
                }
            }
        }

        m_PlayableMeldsWithDraw[0] = tuple(playableMelds, mask);
    }

    void LegalMoveEngine::update_discard(const GameState& gs, const Pile& hand) {
        m_DiscardMask.clear();
        m_DiscardMask.resize(gs.discardPile.size(), false);

        for (int i = 0; i < m_DiscardMask.size(); i++) {
            const auto& card = gs.discardPile.get_card(i);

            std::vector<bool> mask(hand.size());

            std::array<std::vector<uint8_t>, 4> suits;
            std::array<std::vector<uint8_t>, 13> ranks;

            for (int x = 0; x < hand.size(); x++) {
                const auto c = hand.get_card(x);
                suits[static_cast<size_t>(c.suit)].push_back(x);
                ranks[c.value - 1].push_back(x);
            }

            std::vector<std::vector<uint8_t>> playableMelds;

            if (ranks[card.value - 1].size() >= 2) {
                playableMelds.push_back(ranks[card.value - 1]);
                m_DiscardMask[i] = true;
                for (const auto& c : ranks[card.value - 1])
                    mask[c] = true;
            }

            uint8_t up1 = 100, up2 = 100, down1 = 100, down2 = 100;
            for (const auto& c : suits[static_cast<size_t>(card.suit)]) {
                if (const auto val = hand.get_card(c).value; val == card.value + 1) {
                    up1 = c;
                } else if (val == card.value -1) {
                    down1 = c;
                } else if (val == card.value + 2) {
                    up2 = c;
                } else if (val == card.value - 2) {
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
                m_DiscardMask[i] = true;
            }

            if (down1 != 100 && down2 != 100) {
                std::vector<uint8_t> runVector;
                runVector.push_back(down2);
                runVector.push_back(down1);
                playableMelds.push_back(runVector);
                mask[down1] = true;
                mask[down2] = true;
                m_DiscardMask[i] = true;
            }

            if (up1 != 100 && down1 != 100) {
                std::vector<uint8_t> runVector;
                runVector.push_back(down1);
                runVector.push_back(up1);
                playableMelds.push_back(runVector);
                mask[down1] = true;
                mask[up1] = true;
                m_DiscardMask[i] = true;
            }

            m_PlayableMeldsWithDraw[card.get_sort_value()] = tuple(playableMelds, mask);
        }
    }

    std::vector<bool> LegalMoveEngine::get_hand_play_mask(const uint8_t withDraw) {
        return get<1>(m_PlayableMeldsWithDraw[withDraw]);
    }

    std::vector<std::vector<uint8_t>> LegalMoveEngine::get_playable_melds(const uint8_t withDraw) {
        /*auto v = get<0>(m_PlayableMeldsWithDraw[0]);
        if (withDraw)
            v.insert(v.end(), get<0>(m_PlayableMeldsWithDraw[withDraw]).begin(), get<0>(m_PlayableMeldsWithDraw[withDraw]).end());
        return v;*/
        return get<0>(m_PlayableMeldsWithDraw[withDraw]);
    }

    std::vector<bool> LegalMoveEngine::get_discard_pile_mask() const {
        return m_DiscardMask;
    }

} // rummy::utils
