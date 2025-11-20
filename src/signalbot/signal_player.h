//
// Created by somepineaple on 11/12/25.
//

#ifndef DUMMY_SIGNALPLAYER_H
#define DUMMY_SIGNALPLAYER_H

#include <boost/process.hpp>
#include <memory>

#include "../game/player.h"

namespace rummy::clients {
    class signal_player final : public player {
        const std::string phoneNumber;
        void send_user_message(const std::string& message) const;
        void send_game_state(const game_state* gs) const;
        void ask_and_add();
        bool ask_and_discard(game_state* gs);
        std::string receive_user_message();
        std::shared_ptr<boost::process::popen> signalCli;
        std::shared_ptr<boost::asio::io_context> ctx;
    public:
        explicit signal_player(const std::string& playerNumber, const std::string& botNumber);
        bool run_turn(game_state *gs) override;
        [[nodiscard]] std::shared_ptr<player> clone() const override;
        void clean() override;
    };
}


#endif //DUMMY_SIGNALPLAYER_H
