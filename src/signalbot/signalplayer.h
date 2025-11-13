//
// Created by somepineaple on 11/12/25.
//

#ifndef DUMMY_SIGNALPLAYER_H
#define DUMMY_SIGNALPLAYER_H

#include <boost/process.hpp>

#include "../game/player.h"

namespace game::clients {
    class SignalPlayer final : public Player {
        const std::string phoneNumber;
        void sendUserMessage(const std::string& message);
        void sendGameState(const GameState* gs);
        std::string recieveUserMessage();
        boost::process::popen signalCli;
        boost::asio::io_context ctx;
    public:
        explicit SignalPlayer(const std::string& playerNumber, const std::string& botNumber);
        void close();
        bool runTurn(GameState *gs) override;
        //std::shared_ptr<Player> clone() const override;
    };
}


#endif //DUMMY_SIGNALPLAYER_H
