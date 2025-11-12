//
// Created by nj60 on 11/12/25.
//

#include "player.h"

#include <iostream>
#include <numeric>

#include "game.h"

namespace game {
    bool HumanPlayer::runTurn(GameState& gs) {
        bool hasDiscarded = false;
        printGameState(gs);
        printf("You are %s\nType:\n[S] to draw from stock\n[D] to draw from discard\n", name.c_str());

        char response;
        std::cin >> response;
        switch (response) {
            case 's':
            case 'S':
                if (gs.stockPile.size() == 0)
                    return false;
                drawFromStock(gs);
                break;
            case 'd':
            case 'D':
                printf("How many cards would you like to draw?\n");
                std::cin >> response;
                drawFromDiscard(gs, response - '0');
                break;
            default:
                return false;
        }

        do {
            printGameState(gs);
            printf("[P] to play a meld\n[A] to add a card to the meld\n[D] to discard\n[Q] to quit\n");
            std::cin >> response;

            switch (response) {
                case 'D':
                    hasDiscarded = askAndDiscard();
                    if (hasDiscarded)
                        response = 'Q';
                    break;
                case 'P':
                    if (!playWorkingMeld(gs))
                        printf("Cannot play the current meld, it is invalid.");
                    break;
                case 'A':
                    askAndAdd();
                    break;
                case 'Q':
                    break;
                default:
                    return false;
            }
        } while (response != 'Q');

        return hasDiscarded && workingMeld.size() == 0;
    }

    void HumanPlayer::printGameState(const GameState& gs) const {
        printf("Your opponent has %i cards, and has played:\n", gs.opponent->getHandSize());
        gs.opponent->printMelds();

        printf("Discard pile:\n%s\n", gs.discardPile.toString().c_str());
        printf("Your hand:\n%s\n", hand.toString().c_str());
        printf("Current building a meld:\n%s\n", workingMeld.toString().c_str());
        printf("You have played:\n");

        for (auto& m : playedMelds) {
            std::cout << m->toString() << std::endl;
        }
    }

    void Player::drawFromStock(GameState& gs) {
        gs.stockPile.dump(hand, 1);
        hand.sort();
    }

    bool Player::drawFromDiscard(GameState& gs, const unsigned char numCards) {
        if (gs.discardPile.dump(hand, numCards)) {
            hand.sort();
            return true;
        }

        return false;
    }

    bool Player::playWorkingMeld(GameState& gs) {
        if (workingMeld.size() < 3) {
            for (const auto& m : gs.melds) {
                if (workingMeld.tryBuildFrom(m.get())) return true;
            }
        } else if (workingMeld.getMeldType() != INVALID) {
            playedMelds.push_back(std::make_shared<Meld>(workingMeld));
            gs.melds.push_back(playedMelds.back());
            workingMeld = Meld{};
            return true;
        }

        return false;
    }

    unsigned short Player::calcPoints() {
        unsigned char sum = std::accumulate(playedMelds.begin(),playedMelds.end(), 0u, [](const auto& s, const auto& m) {
            return s + m->calcPoints();
        });

        sum -= hand.calcPoints();

        return sum;
    }

    void Player::printMelds() const {
        for (const auto& m : playedMelds) {
            std::cout << m->toString() << std::endl;
        }
    }

    unsigned char Player::getHandSize() const {
        return hand.size();
    }

    bool Player::runTurn(GameState &gs) {
        return false;
    }
} // game
