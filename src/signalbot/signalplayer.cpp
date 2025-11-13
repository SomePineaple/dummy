//
// Created by somepineaple on 11/12/25.
//

#include "signalplayer.h"

#include "nlohmann/json.hpp"
#include <cstdlib>
#include <random>
#include <boost/asio.hpp>

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
    SignalPlayer::SignalPlayer(const std::string& playerNumber, const std::string& botNumber) : 
        phoneNumber(playerNumber), signalCli(ctx, "/usr/bin/signal-cli", {"-a", botNumber, "jsonRpc"}) {

        }

    bool SignalPlayer::runTurn(GameState* gs) {
        // TODO: Have the signal player basically just do the same thing as the human player, but getting responses from the signal bot
        return false;
    }

    void SignalPlayer::sendUserMessage(const std::string& message) {
        int messageId = randomInt();
        json req = {
            {"jsonrpc", "2.0"},
            {"method", "send"},
            "params", {{"message", message}, {"recipient", phoneNumber}},
            {"id", messageId}
        };

        asio::write(signalCli, asio::buffer(req.dump()));

        // Wait for signal-cli to confirm the message was sent.
        bool recievedResult = false;
        while (!recievedResult) {
            std::string response;
            asio::read_until(signalCli, asio::dynamic_buffer(response), '\n');
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

    std::string SignalPlayer::recieveUserMessage() {
        bool recievedMessage = false;
        std::string message;
        while (!recievedMessage) {
            std::string in;
            asio::read_until(signalCli, asio::dynamic_buffer(in), '\n');
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
}
