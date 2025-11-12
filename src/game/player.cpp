//
// Created by nj60 on 11/12/25.
//

#include "player.h"

#include <iostream>
#include <numeric>

#include "game.h"

namespace game {
    bool Player::runTurn(GameState& gs) {
        bool hasDiscarded = false;
        printGameState(gs);
        printf("You are %s\nType:\n[S] to draw from stock\n[D] to draw from discard\n", name.c_str());

        char response;
        std::cin >> response;
        switch (response) {
            case 'S':
                break;
            default:
                return false;
        }

        return hasDiscarded && workingMeld.size() == 0;
    }

    void Player::printGameState(const GameState &gs) const {
        printf("Your opponent has %i cards, and has played:\n", gs.opponent->hand.size());
        for (auto& m : gs.opponent->playedMelds) {
            std::cout << m.toString() << std::endl;
        }

        printf("Discard pile:\n%s\n", gs.discardPile.toString().c_str());
        printf("Your hand:\n%s\n", hand.toString().c_str());
        printf("Current building a meld:\n%s\n", workingMeld.toString().c_str());
        printf("You have played:\n");

        for (auto& m : playedMelds) {
            std::cout << m.toString() << std::endl;
        }
    }

    unsigned short Player::calcPoints() {
        unsigned char sum = std::accumulate(playedMelds.begin(),playedMelds.end(), 0u, [](const auto& s, const Meld& m) {
            return s + m.calcPoints();
        });

        sum -= hand.calcPoints();

        return sum;
    }
} // game
