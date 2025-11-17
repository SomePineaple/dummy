//
// Created by somepineaple on 11/12/25.
//

#ifndef DUMMY_SIGNALPLAYER_H
#define DUMMY_SIGNALPLAYER_H

#include <boost/process.hpp>
#include <memory>

#include "../game/player.h"

namespace game::clients {
    class SignalPlayer final : public Player {
        const std::string phoneNumber;
        void sendUserMessage(const std::string& message) const;
        void sendGameState(const GameState* gs);
        void askAndAdd();
        bool askAndDiscard(GameState* gs);
        std::string receiveUserMessage();
        std::shared_ptr<boost::process::popen> signalCli;
        std::shared_ptr<boost::asio::io_context> ctx;
    public:
        explicit SignalPlayer(const std::string& playerNumber, const std::string& botNumber);
        bool runTurn(GameState *gs) override;
        [[nodiscard]] std::shared_ptr<Player> clone() const override;
        void cleanUp() override;
    };
}


#endif //DUMMY_SIGNALPLAYER_H
