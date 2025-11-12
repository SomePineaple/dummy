//
// Created by nj60 on 11/12/25.
//

#include "HumanPlayer.h"

#include <iostream>
#include "game.h"

namespace game {
    bool HumanPlayer::runTurn(GameState* gs) {
        if (gs == nullptr)
            return false;

        bool hasDiscarded = false;
        printGameState(gs);
        printf("You are %s\nType:\n[S] to draw from stock\n[D] to draw from discard\n", name.c_str());

        char response;
        std::cin >> response;
        switch (response) {
            case 's':
            case 'S':
                if (gs->stockPile.size() == 0)
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
                    hasDiscarded = askAndDiscard(gs);
                    if (hasDiscarded)
                        response = 'Q';
                    else
                        printf("Failed to discard\n");
                    break;
                case 'P':
                    if (!playWorkingMeld(gs))
                        printf("Cannot play the current meld, it is invalid.\n");
                    break;
                case 'A':
                    askAndAdd(gs);
                    break;
                case 'Q':
                    break;
                default:
                    return false;
            }
        } while (response != 'Q');

        return hasDiscarded && workingMeld.size() == 0;
    }

    bool HumanPlayer::askAndDiscard(GameState* gs) {
        printf("Which card number would you like to discard?\n");
        char response;
        std::cin >> response;
        return discard(gs, response - '1');
    }

    void HumanPlayer::askAndAdd(GameState *gs) {
        printf("Which card number would you like to add?\n");
        char response;
        std::cin >> response;
        if (!addToWorkingMeld(response - '1'))
            printf("That was not a valid card number");
    }



    void HumanPlayer::printGameState(const GameState* gs) const {
        printf("Your opponent has %i cards, and has played:\n", gs->opponent->getHandSize());
        gs->opponent->printMelds();

        printf("Discard pile:\n%s\n", gs->discardPile.toString().c_str());
        printf("Your hand:\n%s\n", hand.toString().c_str());
        printf("Current building a meld:\n%s\n", workingMeld.toString().c_str());
        printf("You have played:\n");

        for (auto& m : playedMelds) {
            std::cout << m->toString() << std::endl;
        }
    }
} // game
