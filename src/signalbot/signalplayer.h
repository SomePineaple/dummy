//
// Created by somepineaple on 11/12/25.
//

#ifndef DUMMY_SIGNALPLAYER_H
#define DUMMY_SIGNALPLAYER_H

#include <boost/process.hpp>
#include <memory>

#include "../game/player.h"

namespace rummy::clients {
    class SignalPlayer final : public player {
        const std::string phoneNumber;
        void sendUserMessage(const std::string& message) const;
        void sendGameState(const game_state* gs) const;
        void askAndAdd();
        bool askAndDiscard(game_state* gs);
        std::string receiveUserMessage();
        std::shared_ptr<boost::process::popen> signalCli;
        std::shared_ptr<boost::asio::io_context> ctx;
    public:
        explicit SignalPlayer(const std::string& playerNumber, const std::string& botNumber);
        bool run_turn(game_state *gs) override;
        [[nodiscard]] std::shared_ptr<player> clone() const override;
        void clean() override;
    };
}


#endif //DUMMY_SIGNALPLAYER_H
