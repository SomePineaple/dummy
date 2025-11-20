//
// Created by nj60 on 11/11/25.
//

#include "meld.h"

namespace rummy {
    bool is_valid_set(const std::vector<std::shared_ptr<Card>>& cards) {
        if (cards.size() < 3) return false;

        const auto number = cards[0]->value;
        for (const auto& card : cards) {
            if (card->value != number) return false;
        }

        return true;
    }

    bool is_valid_run(const std::vector<std::shared_ptr<Card>>& cards) {
        if (cards.size() < 3) return false;

        const auto suit = cards[0]->suit;
        for (int i = 1; i < cards.size(); i++) {
            if (cards[i]->suit != suit) return false;
            if (i == cards.size() - 1 && cards[i]->value == 1 && cards[i-1]->value == 13) return true;
            if (cards[i]->value != cards[i-1]->value + 1) return false;
        }

        return true;
    }

    Meld::Meld() {
        this->m_BuildingFrom = nullptr;
    }

    MeldType Meld::get_meld_type() const {
        auto c = m_cards;
        if (m_BuildingFrom != nullptr) {
            c = combine(m_BuildingFrom).get_cards();
        }

        if (is_valid_set(c))
            return SET;
        if (is_valid_run(c))
            return RUN;

        return INVALID;
    }

    bool Meld::try_build_from(Meld* other) {
        this->m_BuildingFrom = other;
        if (get_meld_type() != INVALID)
            return true;
        this->m_BuildingFrom = nullptr;
        return false;
    }
} // game