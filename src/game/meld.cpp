//
// Created by nj60 on 11/11/25.
//

#include "meld.h"

#include <algorithm>

namespace rummy {
    using namespace std;

    bool is_valid_set(const vector<shared_ptr<card>>& cards) {
        if (cards.size() < 3) return false;

        const auto number = cards[0]->value;
        for (const auto& card : cards) {
            if (card->value != number) return false;
        }

        return true;
    }

    bool is_valid_run(const vector<shared_ptr<card>>& cards) {
        if (cards.size() < 3) return false;

        vector cards_mut(cards);
        sort(cards_mut.begin(), cards_mut.end(), [](const shared_ptr<card>& x, const shared_ptr<card>& y) {
            return x->get_sort_value() < y->get_sort_value();
        });
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

    meld_type meld::meld_type() const {
        auto c = cards;
        if (buildingFrom != nullptr) {
            c = combine(buildingFrom).get_cards();
        }

        if (is_valid_set(c))
            return SET;
        if (is_valid_run(c))
            return RUN;

        return INVALID;
    }

    bool meld::try_build_from(meld* buildingFrom) {
        this->buildingFrom = buildingFrom;
        if (meld_type() != INVALID)
            return true;
        this->buildingFrom = nullptr;
        return false;
    }
} // game