//
// Created by somepineaple on 11/12/25.
//

#ifndef DUMMY_SIGNALPLAYER_H
#define DUMMY_SIGNALPLAYER_H

#include <boost/process.hpp>
#include <memory>

#include "../game/clients/player.h"

namespace rummy::clients {
    using namespace std;

    class SignalPlayer final : public Player {
        const string m_PhoneNumber;
        void send_user_message(const std::string& message) const;
        void send_game_state(const GameState& gs) const;
        void ask_and_add();
        bool ask_and_discard(GameState& gs);
        string receive_user_message();
        std::shared_ptr<boost::asio::io_context> m_ctx;
        std::shared_ptr<boost::process::popen> m_SignalCli;
    public:
        explicit SignalPlayer(const std::string& playerNumber, const std::string& botNumber);
        bool run_turn(GameState& gs) override;
        [[nodiscard]] std::shared_ptr<Player> clone() const override;
        void close() override;
        void notify_player(int16_t opponentPoints) override;
    };
}


#endif //DUMMY_SIGNALPLAYER_H
