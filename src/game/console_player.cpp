//
// Created by nj60 on 11/12/25.
//

#include "console_player.h"

#include <iostream>
#include <string>
#include <boost/format.hpp>
#include "game.h"

namespace rummy::clients {
    bool console_player::run_turn(game_state* gs) {
        if (gs == nullptr)
            return false;

        bool hasDiscarded = false;
        print_game_state(gs);
        cout << boost::format("You are %s\nType:\n[S] to draw from stock\n[D] to draw from discard\n") % name << endl;

        char response;
        cin >> response;
        switch (response) {
            case 's':
            case 'S':
                if (!draw_from_stock(gs, 1)) return false;
                cout << "You drew: " << hand.get_card(hand.size() - 1)->to_string() << endl;
                hand.sort();
                break;
            case 'd':
            case 'D':
                cout << "How many cards would you like to draw?\n";
                cin >> response;
                draw_from_discard(gs, response - '0');
                hand.sort();
                break;
            default:
                return false;
        }

        do {
            print_game_state(gs);
            cout << "[P] to play a meld\n[A] to add a card to the meld\n[D] to discard\n[Q] to quit" << endl;
            cin >> response;

            switch (response) {
                case 'D':
                    hasDiscarded = ask_and_discard(gs);
                    if (hasDiscarded)
                        response = 'Q';
                    else
                        cout << "Failed to discard\n";
                    break;
                case 'P':
                    if (!play_working_meld(gs))
                        cout << "Cannot play the current meld, it is invalid.\n";
                    break;
                case 'A':
                    ask_and_add(gs);
                    break;
                case 'Q':
                    break;
                default:
                    return false;
            }
        } while (response != 'Q');

        return hasDiscarded && workingMeld.size() == 0;
    }

    bool console_player::ask_and_discard(game_state* gs) {
        cout << "Which card would you like to discard?" << endl;
        string response;
        cin >> response;

        for (int i = 0; i < hand.size(); i++) {
            if (hand.get_card(i)->to_string() == response)
                return discard(gs, i);
        }

        return false;
    }

    void console_player::ask_and_add(game_state *gs) {
        cout << "Which card would you like to add?" << endl;
        string response;
        cin >> response;
        for (int i = 0; i < hand.size(); i++) {
            if (hand.get_card(i)->to_string() == response) {
                add_to_working_meld(i);
                return;
            }
        }

        cout << "that was not a valid card.\n";
    }

    void console_player::print_game_state(const game_state* gs) const {
        cout << boost::format("Your opponent has %i cards, and has played:\n") % static_cast<int>(gs->opponent->hand_size());
        cout << gs->opponent->print_melds();

        cout << boost::format("Discard pile:\n%s\n") % gs->discardPile.to_string();
        cout << boost::format("Your hand:\n%s\n") % hand.to_string();
        cout << boost::format("Current building a meld:\n%s\n") % workingMeld.to_string();
        cout << "You have played:\n";

        for (auto& m : playedMelds) {
            cout << m->to_string() << ", ";
        }

        cout << endl;
    }

    shared_ptr<player> console_player::clone() const {
        return make_shared<console_player>(*this);
    }
} // game
