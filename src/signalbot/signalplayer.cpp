//
// Created by somepineaple on 11/12/25.
//

#include "signalplayer.h"

#include "nlohmann/json.hpp"

namespace game::clients {
    SignalPlayer::SignalPlayer(const std::string &number) : phoneNumber(number) {}


    bool SignalPlayer::runTurn(GameState* gs) {
        
        return false;
    }
}
