//
// Created by nj60 on 11/11/25.
//

#include "meld.h"

namespace game {
    bool isValidSet(const std::vector<std::shared_ptr<Card>>& cards) {
        if (cards.empty()) return false;

        const auto number = cards[0]->value;
        for (const auto& card : cards) {
            if (card->value != number) return false;
        }

        return true;
    }

    Meld::Meld() {
        this->buildingFrom = std::nullopt;
    }

    Meld::Meld(Meld &buildingFrom) {
        this->buildingFrom = &buildingFrom;
    }

    MeldType Meld::getMeldType() const {
        auto c = cards;
        if (buildingFrom != std::nullopt) {
           // c = combine(buildingFrom.value()).;
            // TODO: Add the building from code.
        }

        if (isValidSet(cards))
            return SET;

        return INVALID;
    }
} // game