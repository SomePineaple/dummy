//
// Created by nj60 on 11/12/25.
//

#include "player.h"

#include <numeric>

#include "game.h"

namespace rummy::clients {
    bool player::draw_from_stock(game_state* gs, const unsigned char numCards) {
        return gs->stockPile.dump(hand, numCards);
    }

    bool player::draw_from_discard(game_state* gs, const unsigned char numCards) {
        if (gs->discardPile.dump(hand, numCards)) {
            if (numCards != 1)
                hand.dump(workingMeld, 1);
            return true;
        }
        return false;
    }

    bool player::play_working_meld(game_state* gs) {
        workingMeld.sort();
        if (workingMeld.size() < 3) {
            for (const auto& m : gs->melds) {
                if (workingMeld.try_build_from(m.get())) return true;
            }
        } else if (workingMeld.meld_type() != INVALID) {
            playedMelds.push_back(std::make_shared<meld>(workingMeld));
            gs->melds.push_back(playedMelds.back());
            workingMeld = meld{};
            return true;
        }

        return false;
    }

    bool player::discard(game_state* gs, const unsigned char cardNumber) {
        if (cardNumber >= hand.size())
            return false;
        gs->discardPile.add_card(hand.get_card(cardNumber));
        hand.remove_card(cardNumber);

        return true;
    }

    bool player::add_to_working_meld(const unsigned char cardNumber) {
        if (cardNumber >= hand.size())
            return false;

        workingMeld.add_card(hand.get_card(cardNumber));
        hand.remove_card(cardNumber);

        return true;
    }


    int player::calc_points() const {
        int sum = std::accumulate(playedMelds.begin(),playedMelds.end(), 0, [](const auto& partial_sum, const auto& meld) {
            return partial_sum + meld->calc_points();
        });

        sum -= hand.calc_points();

        return sum;
    }

    std::string player::print_melds() const {
        std::string meldsStr;
        for (const auto& m : playedMelds) {
            meldsStr += m->to_string() + '\n';
        }

        return meldsStr;
    }

    unsigned char player::hand_size() const {
        return hand.size();
    }

    std::shared_ptr<card> player::get_card(const uint8_t index) const {
        return hand.get_card(index);
    }

    bool player::run_turn(game_state* gs) {
        return false;
    }

    shared_ptr<player> player::clone() const {
        return make_shared<player>(*this);
    }

    void player::clean() {}
} // game
