//
// Created by nj60 on 11/12/25.
//

#include "player.h"

#include <numeric>

#include "game.h"

namespace rummy::clients {
    bool Player::drawFromStock(GameState* gs, const unsigned char numCards) {
        return gs->stockPile.dump(hand, numCards);
    }

    bool Player::drawFromDiscard(GameState* gs, const unsigned char numCards) {
        if (gs->discardPile.dump(hand, numCards)) {
            if (numCards != 1)
                hand.dump(workingMeld, 1);
        }
        return false;
    }

    bool Player::playWorkingMeld(GameState* gs) {
        if (workingMeld.size() < 3) {
            for (const auto& m : gs->melds) {
                if (workingMeld.tryBuildFrom(m.get())) return true;
            }
        } else if (workingMeld.getMeldType() != INVALID) {
            playedMelds.push_back(std::make_shared<Meld>(workingMeld));
            gs->melds.push_back(playedMelds.back());
            workingMeld = Meld{};
            return true;
        }

        return false;
    }

    bool Player::discard(GameState* gs, const unsigned char cardNumber) {
        if (cardNumber >= hand.size())
            return false;
        gs->discardPile.addCard(hand.getCard(cardNumber));
        hand.removeCard(cardNumber);

        return true;
    }

    bool Player::addToWorkingMeld(const unsigned char cardNumber) {
        if (cardNumber >= hand.size())
            return false;

        workingMeld.addCard(hand.getCard(cardNumber));
        hand.removeCard(cardNumber);

        return true;
    }


    unsigned short Player::calcPoints() {
        unsigned char sum = std::accumulate(playedMelds.begin(),playedMelds.end(), 0u, [](const auto& s, const auto& m) {
            return s + m->calcPoints();
        });

        sum -= hand.calcPoints();

        return sum;
    }

    std::string Player::printMelds() const {
        std::string meldsStr;
        for (const auto& m : playedMelds) {
            meldsStr += m->toString() + '\n';
        }

        return meldsStr;
    }

    unsigned char Player::getHandSize() const {
        return hand.size();
    }

    bool Player::runTurn(GameState* gs) {
        return false;
    }

    shared_ptr<Player> Player::clone() const {
        return make_shared<Player>(*this);
    }

    void Player::cleanUp(){}
} // game
