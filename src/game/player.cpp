//
// Created by nj60 on 11/12/25.
//

#include "player.h"

#include <numeric>

#include "game.h"

namespace rummy::clients {
    bool player::drawFromStock(game_state* gs, const unsigned char numCards) {
        return gs->stockPile.dump(hand, numCards);
    }

    bool player::draw_from_discard(game_state* gs, const unsigned char numCards) {
        if (gs->discardPile.dump(hand, numCards)) {
            if (numCards != 1)
                hand.dump(workingMeld, 1);
        }
        return false;
    }

    bool player::play_working_meld(game_state* gs) {
        if (workingMeld.size() < 3) {
            for (const auto& m : gs->melds) {
                if (workingMeld.tryBuildFrom(m.get())) return true;
            }
        } else if (workingMeld.getMeldType() != INVALID) {
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


    unsigned short player::calcPoints() {
        unsigned char sum = std::accumulate(playedMelds.begin(),playedMelds.end(), 0u, [](const auto& s, const auto& m) {
            return s + m->calc_points();
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

    bool player::run_turn(game_state* gs) {
        return false;
    }

    shared_ptr<player> player::clone() const {
        return make_shared<player>(*this);
    }

    void player::clean(){}
} // game
