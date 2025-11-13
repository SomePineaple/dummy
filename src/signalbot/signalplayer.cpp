//
// Created by somepineaple on 11/12/25.
//

#include "signalplayer.h"

#include "nlohmann/json.hpp"
#include <cstdlib>
#include <format>
#include <memory>
#include <random>
#include <boost/asio.hpp>
#include <stdexcept>
#include "../game/game.h"

using json = nlohmann::json;
namespace asio = boost::asio;

int randomInt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 10000);

    return distrib(gen);
}

namespace game::clients {
    // TODO: Fix hardcoded signal-cli executable location
    SignalPlayer::SignalPlayer(const std::string& playerNumber, const std::string& botNumber) : phoneNumber(playerNumber) {
        ctx = make_shared<asio::io_context>();

        boost::process::popen* p = new boost::process::popen(*ctx, "/usr/bin/signal-cli", {"-a", botNumber, "jsonRpc"});

        signalCli = shared_ptr<boost::process::popen>(p);

        sendUserMessage("Someone would like to play a game of Rummy with you!");
    }

    bool SignalPlayer::runTurn(GameState* gs) {
        if (gs == nullptr)
            return false;

        bool hasDiscarded = false;
        sendGameState(gs);
        sendUserMessage("Send:\n[Stock] to draw from stock\n[Discard] to draw from discard");
        string userResponse = recieveUserMessage();
        if (userResponse == "Stock") {
            if (!drawFromStock(gs, 1)) return false;
        } else if (userResponse == "Discard") {
            sendUserMessage("How many cards would you like to draw?");
            string reply = recieveUserMessage();
            try {
                if (!drawFromDiscard(gs, atoi(reply.c_str()))) throw out_of_range("To big");
            } catch (const exception&) {
                sendUserMessage("You didn't provide a valid number");
                return false;
            }
        }

        bool loop = true;
        do {
            sendGameState(gs);
            sendUserMessage("Send:\n[Play] to play the meld\n[Add] to add a card to the meld\n[Discard] to discard\n[Reset] to reset your turn (cheater)");
            string userResponse = recieveUserMessage();
            if (userResponse == "Play") {
                if (!playWorkingMeld(gs))
                    sendUserMessage("Can't play the working meld, it is no valid.");
            } else if (userResponse == "Add") {
                askAndAdd();
            } else if (userResponse == "Discard") {
                if (workingMeld.size() != 0)
                    sendUserMessage("You cannot discard yet, you are building a meld to play!");
                else if (askAndDiscard(gs))
                    return true;
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
        string userResponse = recieveUserMessage();
        for (int i = 0; i < hand.size(); i++) {
            if (hand.getCard(i)->toString() == userResponse) {
                return discard(gs, i);
            }
        }

        return false;
    }

    void SignalPlayer::askAndAdd() {
        sendUserMessage("Which card would you like to add?");
        string userResponse = recieveUserMessage();
        for (int i = 0; i < hand.size(); i++) {
            if (hand.getCard(i)->toString() == userResponse) {
                if (!addToWorkingMeld(i))
                    sendUserMessage("That was not a valid card.");
            }
        }
    }

    void SignalPlayer::sendGameState(const GameState* gs) {
        string message;
        message += std::format("Your opponent has %i cards, and has played:\n", gs->opponent->getHandSize());
        message += gs->opponent->printMelds();
        message += std::format("Discard pile:\n%s\n", gs->discardPile.toString());
        message += std::format("Your hand:\n%s\n", hand.toString());
        message += std::format("Current building a meld:\n%s\n", workingMeld.toString());
        message += std::format("You have played:\n%s\n", printMelds());

        sendUserMessage(message);
    }

    void SignalPlayer::sendUserMessage(const std::string& message) {
        int messageId = randomInt();
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "send"},
            "params", {{"message", message}, {"recipient", phoneNumber}},
            {"id", messageId}
        };

        asio::write(*signalCli, asio::buffer(req.dump()));

        // Wait for signal-cli to confirm the message was sent.
        bool recievedResult = false;
        while (!recievedResult) {
            string response;
            asio::read_until(*signalCli, asio::dynamic_buffer(response), '\n');
            auto res = json::parse(response);
            if (res.contains("id") && res["id"] == messageId && res.contains("result")) {
                auto result = res["result"]["results"][0]["type"];
                if (!(result == "SUCCESS")) {
                    printf("Failed to send message to user: got response %s\n", response.c_str());
                    exit(1);
                } else {
                    recievedResult = true;
                }
            } else {
                printf("got response that doesn't make sense: %s\n", response.c_str());
            }
        }
    }

    // Waits for the user to send a proper dataMessage and returns it
    std::string SignalPlayer::recieveUserMessage() {
        bool recievedMessage = false;
        string message;
        while (!recievedMessage) {
            string in;
            asio::read_until(*signalCli, asio::dynamic_buffer(in), '\n');
            auto res = json::parse(in);
            if (res.contains("method") && res["method"] == "receive" && res["params"]["envelope"].contains("dataMessage")) {
                auto message = res["params"]["envelope"]["dataMessage"];
                if (message["message"].is_string()) {
                    message = message["message"];
                    recievedMessage = true;
                }
            }
        }

        return message;
    }

    std::shared_ptr<Player> SignalPlayer::clone() const {
        return make_shared<SignalPlayer>(*this);
    }

    void SignalPlayer::cleanUp() {
        printf("Closing signal CLI...\n");
        signalCli->request_exit();
        ctx->stop();
        signalCli->wait();
        printf("Signal CLI closed\n");
    }
}
