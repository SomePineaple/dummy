//
// Created by somepineaple on 11/12/25.
//

#include "signalplayer.h"

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

int randomInt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 10000);

    return distrib(gen);
}

namespace rummy::clients {
    SignalPlayer::SignalPlayer(const std::string& playerNumber, const std::string& botNumber) : phoneNumber(playerNumber) {
        ctx = make_shared<asio::io_context>();

        // I don't know why, but make_shared just doesn't work here, it can't find the contrstructo
        signalCli = shared_ptr<bp::popen>(new bp::popen(*ctx, env::find_executable("signal-cli"), {"-a", botNumber, "jsonRpc"}));

        sendUserMessage("Someone would like to play a game of Rummy with you! (respond [kill] to any prompt to stop the game)");
    }

    bool SignalPlayer::runTurn(GameState* gs) {
        if (gs == nullptr)
            return false;

        bool hasDiscarded = false;
        bool hasDrawn = false;
        do {
            sendGameState(gs);
            sendUserMessage("Send:\n[Stock] to draw from stock\n[Discard] to draw from discard");
            string userResponse = receiveUserMessage();
            if (userResponse == "Stock") {
                if (!drawFromStock(gs, 1)) return false;
                else {
                    sendUserMessage((boost::format("You just drew %s") % hand.getCards().back()->toString()).str());
                    hand.sort();
                    hasDrawn = true;
                }
            } else if (userResponse == "Discard") {
                sendUserMessage("How many cards would you like to draw?");
                string reply = receiveUserMessage();
                try {
                    if (!drawFromDiscard(gs, atoi(reply.c_str()))) throw out_of_range("To big");
                    else hasDrawn = true;
                } catch (const exception&) {
                    sendUserMessage("You didn't provide a valid number");
                    return false;
                }
            } else {
                sendUserMessage("That was not a valid response");
            }
        } while (!hasDrawn);

        bool loop = true;
        do {
            sendGameState(gs);
            sendUserMessage("Send:\n[Play] to play the meld\n[Add] to add a card to the meld\n[Discard] to discard\n[Reset] to reset your turn (cheater)");
            string userResponse = receiveUserMessage();
            if (userResponse == "Play") {
                if (!playWorkingMeld(gs))
                    sendUserMessage("Can't play the working meld, it is no valid.");
            } else if (userResponse == "Add") {
                askAndAdd();
            } else if (userResponse == "Discard") {
                if (workingMeld.size() != 0)
                    sendUserMessage("You cannot discard yet, you are building a meld to play!");
                else if (askAndDiscard(gs)) {
                    sendUserMessage("Your opponent is playing...");
                    return true;
                }
            } else if (userResponse == "Reset") {
                loop = false;
            } else {
                sendUserMessage("That was not a valid response. Try again");
            }
        } while (loop);

        return false;
    }

    bool SignalPlayer::askAndDiscard(GameState* gs) {
        sendUserMessage("Which card would you like to discard?");
        string userResponse = receiveUserMessage();
        for (int i = 0; i < hand.size(); i++) {
            if (hand.getCard(i)->toString() == userResponse) {
                return discard(gs, i);
            }
        }

        return false;
    }

    void SignalPlayer::askAndAdd() {
        sendUserMessage("Which card would you like to add?");
        string userResponse = receiveUserMessage();
        for (int i = 0; i < hand.size(); i++) {
            if (hand.getCard(i)->toString() == userResponse) {
                if (!addToWorkingMeld(i))
                    sendUserMessage("That was not a valid card.");
            }
        }
    }

    void SignalPlayer::sendGameState(const GameState* gs) const {
        string message;
        message += (boost::format("Your opponent has %i cards, and has played:\n") % static_cast<int>(gs->opponent->getHandSize())).str();
        message += gs->opponent->printMelds();
        message += (boost::format("Discard pile:\n%s\n") % gs->discardPile.toString()).str();
        message += (boost::format("Your hand:\n%s\n") % hand.toString()).str();
        message += (boost::format("Current building a meld:\n%s\n") % workingMeld.toString()).str();
        message += (boost::format("You have played:\n%s\n") % printMelds()).str();

        sendUserMessage(message);
    }

    void SignalPlayer::sendUserMessage(const string& message) const {
        int messageId = randomInt();
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "send"},
            {"params", {{"message", message}, {"recipient", phoneNumber}, {"expiresInSeconds", 3600}}},
            {"id", messageId}
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
    std::string SignalPlayer::receiveUserMessage() {
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
            cleanUp();
            exit(0);
        }

        return message;
    }

    std::shared_ptr<Player> SignalPlayer::clone() const {
        return make_shared<SignalPlayer>(*this);
    }

    void SignalPlayer::cleanUp() {
        sendUserMessage("Stopping server...");
        cout << "Closing signal CLI..." << endl;
        signalCli->request_exit();
        ctx->stop();
        signalCli->wait();
        cout << "Signal CLI closed" << endl;
    }
}
