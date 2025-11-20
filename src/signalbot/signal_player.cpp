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
#include "../game/game.h"

using json = nlohmann::json;
namespace asio = boost::asio;
namespace bp = boost::process;
namespace env = bp::environment;

int rand_int() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 10000);

    return distrib(gen);
}

namespace rummy::clients {
    signal_player::signal_player(const std::string& playerNumber, const std::string& botNumber) : phoneNumber(playerNumber) {
        ctx = make_shared<asio::io_context>();

        // I don't know why, but make_shared just doesn't work here, it can't find the contrstructor
        signalCli = shared_ptr<bp::popen>(new bp::popen(*ctx, env::find_executable("signal-cli"), {"-a", botNumber, "jsonRpc"}));

        send_user_message("Someone would like to play a game of Rummy with you! (respond [kill] to any prompt to stop the game)");
    }

    bool signal_player::run_turn(game_state* gs) {
        if (gs == nullptr)
            return false;

        bool drawn = false;
        do {
            send_game_state(gs);
            send_user_message("Send:\n[Stock] to draw from stock\n[Discard] to draw from discard");
            if (string response = receive_user_message(); response == "Stock") {
                if (!draw_from_stock(gs, 1)) return false;

                send_user_message((boost::format("You just drew %s") % hand.get_cards().back()->to_string()).str());
                hand.sort();
                drawn = true;
            } else if (response == "Discard") {
                send_user_message("How many cards would you like to draw?");
                string reply = receive_user_message();
                try {
                    if (!draw_from_discard(gs, atoi(reply.c_str()))) throw out_of_range("To big");
                    hand.sort();
                    drawn = true;
                } catch (const exception&) {
                    send_user_message("You didn't provide a valid number");
                    return false;
                }
            } else {
                send_user_message("That was not a valid response");
            }
        } while (!drawn);

        bool loop = true;
        do {
            send_game_state(gs);
            send_user_message("Send:\n[Play] to play the meld\n[Add] to add a card to the meld\n[Discard] to discard\n[Reset] to reset your turn (cheater)");
            if (string response = receive_user_message();response == "Play") {
                if (!play_working_meld(gs))
                    send_user_message("Can't play the working meld, it is no valid.");
            } else if (response == "Add") {
                ask_and_add();
            } else if (response == "Discard") {
                if (workingMeld.size() != 0)
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

    bool signal_player::ask_and_discard(game_state* gs) {
        send_user_message("Which card would you like to discard?");
        string userResponse = receive_user_message();
        for (int i = 0; i < hand.size(); i++) {
            if (hand.get_card(i)->to_string() == userResponse) {
                return discard(gs, i);
            }
        }

        return false;
    }

    void signal_player::ask_and_add() {
        send_user_message("Which card would you like to add?");
        string userResponse = receive_user_message();
        for (int i = 0; i < hand.size(); i++) {
            if (hand.get_card(i)->to_string() == userResponse) {
                if (!add_to_working_meld(i))
                    send_user_message("That was not a valid card.");
            }
        }
    }

    void signal_player::send_game_state(const game_state* gs) const {
        string message;
        message += (boost::format("Your opponent has %i cards, and has played:\n") % static_cast<int>(gs->opponent->hand_size())).str();
        message += gs->opponent->print_melds();
        message += (boost::format("Discard pile:\n%s\n") % gs->discardPile.to_string()).str();
        message += (boost::format("Your hand:\n%s\n") % hand.to_string()).str();
        message += (boost::format("Current building a meld:\n%s\n") % workingMeld.to_string()).str();
        message += (boost::format("You have played:\n%s\n") % print_melds()).str();

        send_user_message(message);
    }

    void signal_player::send_user_message(const string& message) const {
        int id = rand_int();
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "send"},
            {"params", {{"message", message}, {"recipient", phoneNumber}, {"expiresInSeconds", 3600}}},
            {"id", id}
        };

        cout << "Sending message..." << endl;

        asio::write(*signalCli, asio::buffer(req.dump() + '\n'));

        // Wait for signal-cli to confirm the message was sent.
        bool receivedResult = false;
        while (!receivedResult) {
            cout << "awaiting response..." << endl;
            string response;
            asio::read_until(*signalCli, asio::dynamic_buffer(response), "}\n");
            json res;
            try {
                res = json::parse(response);
            } catch (const exception& e) {
                cout << boost::format("Failed to parse response. Err: %s\n Response: %s") % e.what() % response << endl;
                continue;
            }
            if (res.contains("id") && res["id"] == id && res.contains("result")) {
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
    std::string signal_player::receive_user_message() {
        bool receivedMessage = false;
        string message;
        while (!receivedMessage) {
            string in;
            asio::read_until(*signalCli, asio::dynamic_buffer(in), '\n');
            auto res = json::parse(in);
            if (res.contains("method") && res["method"] == "receive" && res["params"]["envelope"].contains("dataMessage")) {
                auto envelope = res["params"]["envelope"];
                if (envelope["dataMessage"]["message"].is_string() && envelope["sourceNumber"] == phoneNumber) {
                    message = envelope["dataMessage"]["message"];
                    receivedMessage = true;
                }
            }
        }

        if (message == "kill") {
            clean();
            exit(0);
        }

        return message;
    }

    std::shared_ptr<player> signal_player::clone() const {
        return make_shared<signal_player>(*this);
    }

    void signal_player::clean() {
        send_user_message("Stopping server...");
        cout << "Closing signal CLI..." << endl;
        signalCli->request_exit();
        ctx->stop();
        signalCli->wait();
        cout << "Signal CLI closed" << endl;
    }
}
