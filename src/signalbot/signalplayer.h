//
// Created by somepineaple on 11/12/25.
//

#ifndef DUMMY_SIGNALPLAYER_H
#define DUMMY_SIGNALPLAYER_H

#include "../game/player.h"

namespace game::clients {
    class SignalPlayer final : public Player {
        const std::string phoneNumber;
    public:
        explicit SignalPlayer(const std::string& number);
        bool runTurn(GameState *gs) override;
        std::shared_ptr<Player> clone() const override;
    };
}


#endif //DUMMY_SIGNALPLAYER_H
