//
// Created by nj60 on 11/11/25.
//

#include "meld.h"

namespace rummy {
    bool isValidSet(const std::vector<std::shared_ptr<Card>>& cards) {
        if (cards.size() < 3) return false;

        const auto number = cards[0]->value;
        for (const auto& card : cards) {
            if (card->value != number) return false;
        }

        return true;
    }

    bool isValidRun(const std::vector<std::shared_ptr<Card>>& cards) {
        if (cards.size() < 3) return false;

        const auto suit = cards[0]->suit;
        for (int i = 1; i < cards.size(); i++) {
            if (cards[i]->suit != suit) return false;
            if (i == cards.size() - 1 && cards[i]->value == 1 && cards[i-1]->value == 13) return true;
            if (cards[i]->value != cards[i-1]->value + 1) return false;
        }

        return true;
    }

    meld::meld() {
        this->buildingFrom = nullptr;
    }

    meld_type meld::getMeldType() const {
        auto c = cards;
        if (buildingFrom != nullptr) {
            c = combine(buildingFrom).getCards();
        }

        if (isValidSet(c))
            return SET;
        if (isValidRun(c))
            return RUN;

        return INVALID;
    }

    bool meld::tryBuildFrom(meld* buildingFrom) {
        this->buildingFrom = buildingFrom;
        if (getMeldType() != INVALID)
            return true;
        this->buildingFrom = nullptr;
        return false;
    }
} // game