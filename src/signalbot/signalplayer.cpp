//
// Created by somepineaple on 11/12/25.
//

#include "signalplayer.h"

#include "nlohmann/json.hpp"

namespace game::clients {
    SignalPlayer::SignalPlayer(const std::string &number) : phoneNumber(number) {}


    bool SignalPlayer::runTurn(GameState* gs) {
        // TODO: Have the signal player basically just do the same thing as the human player, but getting responses from the signal bot
        return false;
    }
}
