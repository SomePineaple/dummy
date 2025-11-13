//
// Created by nj60 on 11/12/25.
//

#include "humanplayer.h"

#include <iostream>
#include <string>
#include <format>
#include "game.h"

namespace game::clients {
    bool HumanPlayer::runTurn(GameState* gs) {
        if (gs == nullptr)
            return false;

        bool hasDiscarded = false;
        printGameState(gs);
        cout << format("You are {}\nType:\n[S] to draw from stock\n[D] to draw from discard\n", name) << endl;

        char response;
        cin >> response;
        switch (response) {
            case 's':
            case 'S':
                if (!drawFromStock(gs, 1)) return false;
                cout << "You drew: " << hand.getCard(hand.size() - 1)->toString() << endl;
                hand.sort();
                break;
            case 'd':
            case 'D':
                cout << "How many cards would you like to draw?\n";
                cin >> response;
                drawFromDiscard(gs, response - '0');
                hand.sort();
                break;
            default:
                return false;
        }

        do {
            printGameState(gs);
            cout << "[P] to play a meld\n[A] to add a card to the meld\n[D] to discard\n[Q] to quit" << endl;
            cin >> response;

            switch (response) {
                case 'D':
                    hasDiscarded = askAndDiscard(gs);
                    if (hasDiscarded)
                        response = 'Q';
                    else
                        cout << "Failed to discard\n";
                    break;
                case 'P':
                    if (!playWorkingMeld(gs))
                        cout << "Cannot play the current meld, it is invalid.\n";
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
        cout << "Which card would you like to discard?" << endl;
        string response;
        cin >> response;

        for (int i = 0; i < hand.size(); i++) {
            if (hand.getCard(i)->toString() == response)
                return discard(gs, i);
        }

        return false;
    }

    void HumanPlayer::askAndAdd(GameState *gs) {
        cout << "Which card would you like to add?" << endl;
        string response;
        cin >> response;
        for (int i = 0; i < hand.size(); i++) {
            if (hand.getCard(i)->toString() == response) {
                addToWorkingMeld(i);
                return;
            }
        }

        cout << "that was not a valid card.\n";
    }

    void HumanPlayer::printGameState(const GameState* gs) const {
        cout << format("Your opponent has {} cards, and has played:\n", gs->opponent->getHandSize());
        cout << gs->opponent->printMelds();

        cout << format("Discard pile:\n{}\n", gs->discardPile.toString());
        cout << format("Your hand:\n{}\n", hand.toString());
        cout << format("Current building a meld:\n{}\n", workingMeld.toString());
        cout << format("You have played:\n");

        for (auto& m : playedMelds) {
            cout << m->toString() << ", ";
        }

        cout << endl;
    }

    shared_ptr<Player> HumanPlayer::clone() const {
        return make_shared<HumanPlayer>(*this);
    }
} // game
