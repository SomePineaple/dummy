//
// Created by somepineaple on 11/12/25.
//

#include "signal_player.h"

#include "nlohmann/json.hpp"
#include <boost/process/v2/environment.hpp>
#include <iostream>
#include <boost/format.hpp>
#include <memory>
#include <random>
#include <boost/asio.hpp>
#include <stdexcept>
#include <utility>
#include "../game/game.h"

using json = nlohmann::json;
namespace ba = boost::asio;
namespace bp = boost::process;
namespace be = bp::environment;

using namespace std;

int rand_int() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(1, 10000);

    return distrib(gen);
}

namespace rummy::clients {
    SignalPlayer::SignalPlayer(string  playerNumber, const string& botNumber)
    : m_PhoneNumber(std::move(playerNumber)), m_ctx(make_shared<ba::io_context>()), m_SignalCli(new bp::popen(*m_ctx, be::find_executable("signal-cli"), {"-a", botNumber, "jsonRpc"}))
    {
        send_user_message("Someone would like to play a game of Rummy with you! (respond [kill] to any prompt to stop the game)");
    }

    bool SignalPlayer::run_turn(GameState& gs) {
        bool hasDrawn = false;
        do {
            send_game_state(gs);
            send_user_message("Send:\n[Stock] to draw from stock\n[Discard] to draw from discard");
            if (const string userResponse = receive_user_message(); userResponse == "Stock") {
                if (!draw_from_stock(gs, 1)) return false;

                send_user_message((boost::format("You just drew %s") % m_hand.get_cards().back().to_string()).str());
                m_hand.sort();
                hasDrawn = true;
            } else if (userResponse == "Discard") {
                send_user_message("How many cards would you like to draw?");
                string reply = receive_user_message();
                try {
                    if (!draw_from_discard(gs, atoi(reply.c_str()))) throw out_of_range("To big");
                    hasDrawn = true;
                } catch (const exception&) {
                    send_user_message("You didn't provide a valid number");
                    return false;
                }
            } else {
                send_user_message("That was not a valid response");
            }
        } while (!hasDrawn);

        bool loop = true;
        do {
            send_game_state(gs);
            send_user_message("Send:\n[Play] to play the meld\n[Add] to add a card to the meld\n[Discard] to discard\n[Reset] to reset your turn (cheater)");
            if (string response = receive_user_message(); response == "Play") {
                if (!play_working_meld(gs))
                    send_user_message("Can't play the working meld, it is no valid.");
            } else if (response == "Add") {
                ask_and_add();
            } else if (response == "Discard") {
                if (m_WorkingMeld.size() != 0)
                    send_user_message("You cannot discard yet, you are building a meld to play!");
                else if (ask_and_discard(gs)) {
                    send_user_message("Your opponent is playing...");
                    return true;
                }
            } else if (response == "Reset") {
                loop = false;
            } else {
                send_user_message("That was not a valid response. Try again");
            }
        } while (loop);

        return false;
    }

    bool SignalPlayer::ask_and_discard(GameState& gs) {
        send_user_message("Which card would you like to discard?");
        string userResponse = receive_user_message();
        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i).to_string() == userResponse) {
                return discard(gs, i);
            }
        }

        return false;
    }

    void SignalPlayer::ask_and_add() {
        send_user_message("Which card would you like to add?");
        const string userResponse = receive_user_message();
        for (int i = 0; i < m_hand.size(); i++) {
            if (m_hand.get_card(i).to_string() == userResponse) {
                if (!add_to_working_meld(i))
                    send_user_message("That was not a valid card.");
            }
        }
    }

    void SignalPlayer::send_game_state(const GameState& gs) const {
        string message;
        message += (boost::format("Your opponent has %i cards, and has played:\n") % static_cast<int>(gs.opponent->get_hand_size())).str();
        message += gs.opponent->print_melds();
        message += (boost::format("Discard pile:\n%s\n") % gs.discardPile.to_string()).str();
        message += (boost::format("Your hand:\n%s\n") % m_hand.to_string()).str();
        message += (boost::format("Current building a meld:\n%s\n") % m_WorkingMeld.to_string()).str();
        message += (boost::format("You have played:\n%s\n") % print_melds()).str();

        send_user_message(message);
    }

    void SignalPlayer::send_user_message(const string& message) const {
        int messageId = rand_int();
        const json req = {
            {"jsonrpc", "2.0"},
            {"method", "send"},
            {"params", {{"message", message}, {"recipient", m_PhoneNumber}, {"expiresInSeconds", 3600}}},
            {"id", messageId}
        };

        cout << "Sending message..." << endl;

        ba::write(*m_SignalCli, ba::buffer(req.dump() + '\n'));

        // Wait for signal-cli to confirm the message was sent.
        bool receivedResult = false;
        while (!receivedResult) {
            cout << "awaiting response..." << endl;
            string response;
            ba::read_until(*m_SignalCli, ba::dynamic_buffer(response), "}\n");
            json res;
            try {
                res = json::parse(response);
            } catch (const exception& e) {
                cout << boost::format("Failed to parse response. Err: %s\n Response: %s") % e.what() % response << endl;
                continue;
            }
            if (res.contains("id") && res["id"] == messageId && res.contains("result")) {
                auto result = res["result"]["results"][0]["type"];
                if (result != "SUCCESS") {
                    cout << boost::format("Failed to send message to user: got response %s") % response << endl;
                    exit(1);
                }

                receivedResult = true;
                cout << "got response" << endl;
            } else {
                cout << boost::format("got response that doesn't make sense: %s") % response << endl;
            }
        }
    }

    // Waits for the user to send a proper dataMessage and returns it
    string SignalPlayer::receive_user_message() {
        bool receivedMessage = false;
        string message;
        while (!receivedMessage) {
            string in;
            ba::read_until(*m_SignalCli, ba::dynamic_buffer(in), '\n');
            auto res = json::parse(in);
            if (res.contains("method") && res["method"] == "receive" && res["params"]["envelope"].contains("dataMessage")) {
                auto envelope = res["params"]["envelope"];
                if (envelope["dataMessage"]["message"].is_string() && envelope["sourceNumber"] == m_PhoneNumber) {
                    message = envelope["dataMessage"]["message"];
                    receivedMessage = true;
                }
            }
        }

        if (message == "kill") {
            close();
            exit(0);
        }

        return message;
    }

    shared_ptr<Player> SignalPlayer::clone() const {
        return make_shared<SignalPlayer>(*this);
    }

    void SignalPlayer::notify_player(int16_t opponentPoints) {
        send_user_message((boost::format("The game is over. You have %i points, and your opponent has %i points.") % calc_points() % opponentPoints).str());
    }

    void SignalPlayer::close() {
        send_user_message("Stopping server...");
        cout << "Closing signal CLI..." << endl;
        m_SignalCli->request_exit();
        m_ctx->stop();
        m_SignalCli->wait();
        cout << "Signal CLI closed" << endl;
    }
}
