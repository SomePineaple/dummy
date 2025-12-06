//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_HUMANPLAYER_H
#define DUMMY_HUMANPLAYER_H

#include "player.h"

namespace rummy::clients {
    using namespace std;
    class ConsolePlayer final : public Player {
        string m_name;
        void print_game_state(const GameState& gs) const;
        bool ask_and_discard(GameState& gs);
        void ask_and_add();
    public:
        explicit ConsolePlayer(const string&  n) : m_name(n) {}

        bool run_turn(GameState& gs) override;
        [[nodiscard]] shared_ptr<Player> clone() const override;
        void notify_player(int16_t opponentPoints) override;
    };
} // game

#endif //DUMMY_HUMANPLAYER_H
