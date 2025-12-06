//
// Created by nj60 on 11/12/25.
//

#include "console_player.h"

#include <iostream>
#include <string>
#include <boost/format.hpp>
#include "../game.h"

namespace rummy::clients {
    bool ConsolePlayer::run_turn(GameState& gs) {
        bool hasDiscarded = false;
        print_game_state(gs);
        cout << boost::format("You are %s\nType:\n[S] to draw from stock\n[D] to draw from discard\n") % m_name << endl;

        char response;
        cin >> response;
        switch (response) {
            case 's':
            case 'S':
                if (!draw_from_stock(gs, 1)) return false;
                cout << "You drew: " << m_hand.get_card(m_hand.size() - 1).to_string() << endl;
                m_hand.sort();
                break;
            case 'd':
            case 'D':
                cout << "How many cards would you like to draw?\n";
                cin >> response;
                draw_from_discard(gs, response - '0');
                m_hand.sort();
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
                    ask_and_add();
                    break;
                case 'Q':
                    break;
                default:
                    return false;
            }
        } while (response != 'Q');

        return hasDiscarded && m_WorkingMeld.size() == 0;
    }

    bool ConsolePlayer::ask_and_discard(GameState& gs) {
        cout << "Which card would you like to discard?" << endl;
        string response;
        cin >> response;

        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i).to_string() == response)
                return discard(gs, i);
        }

        return false;
    }

    void ConsolePlayer::ask_and_add() {
        cout << "Which card would you like to add?" << endl;
        string response;
        cin >> response;
        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i).to_string() == response) {
                add_to_working_meld(i);
                return;
            }
        }

        cout << "that was not a valid card.\n";
    }

    void ConsolePlayer::print_game_state(const GameState& gs) const {
        cout << boost::format("Your opponent has %i cards, and has played:\n") % static_cast<int>(gs.opponent->get_hand_size());
        cout << gs.opponent->print_melds();

        cout << boost::format("Discard pile:\n%s\n") % gs.discardPile.to_string();
        cout << boost::format("Your hand:\n%s\n") % m_hand.to_string();
        cout << boost::format("Current building a meld:\n%s\n") % m_WorkingMeld.to_string();
        cout << "You have played:\n";

        for (auto& m : m_PlayedMelds) {
            cout << m->to_string() << ", ";
        }

        cout << endl;
    }

    shared_ptr<Player> ConsolePlayer::clone() const {
        return make_shared<ConsolePlayer>(*this);
    }

    void ConsolePlayer::notify_player(int16_t opponentPoints) {
        cout << boost::format("The game is over. You have %i points, and your opponent has %i points.") % calc_points() % opponentPoints << endl;
    }
} // game
