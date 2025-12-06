//
// Created by nj60 on 11/11/25.
//

#include "meld.h"
#include <algorithm>

namespace rummy {
    bool is_valid_set(const vector<Card>& cards) {
        if (cards.size() < 3) return false;

        const auto number = cards[0].value;
        for (const auto& card : cards) {
            if (card.value != number) return false;
        }

        return true;
    }

    bool is_valid_run(const vector<Card>& cards) {
        if (cards.size() < 3) return false;

        vector cardsMut(cards);
        sort(cardsMut.begin(), cardsMut.end(), [](const auto& c1, const auto& c2) {
            return c1.get_sort_value() < c2.get_sort_value();
        });

        // If we have a high ace we will put it at the back.
        if (cardsMut[0].value == 1 && cardsMut[1].value != 2) {
            cardsMut.push_back(cardsMut.front());
            cardsMut.erase(cardsMut.begin());
        }

        const auto suit = cardsMut[0].suit;
        for (int i = 1; i < cards.size(); i++) {
            if (cardsMut[i].suit != suit) return false;
            if (i == cards.size() - 1 && cardsMut[i].value == 1 && cardsMut[i-1].value == 13) return true;
            if (cardsMut[i].value != cardsMut[i-1].value + 1) return false;
        }

        return true;
    }

    Meld::Meld() {
        this->m_BuildingFrom = nullptr;
    }

    MeldType Meld::get_meld_type() const {
        const auto c = get_cards();

        if (is_valid_set(c))
            return SET;
        if (is_valid_run(c))
            return RUN;

        return INVALID;
    }

    bool Meld::try_build_from(Meld* other) {
        m_BuildingFrom = other;
        if (get_meld_type() != INVALID)
            return true;

        m_BuildingFrom = nullptr;
        return false;
    }

    std::vector<Card> Meld::get_cards() const {
        std::vector<Card> cards;
        if (m_BuildingFrom != nullptr) {
            auto buildFromCards = m_BuildingFrom->get_cards();
            cards.insert(cards.end(), buildFromCards.begin(), buildFromCards.end());
        }

        cards.insert(cards.end(), m_cards.begin(), m_cards.end());
        return cards;
    }
} // game
