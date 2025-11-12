//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_HUMANPLAYER_H
#define DUMMY_HUMANPLAYER_H

#include "player.h"

namespace game::clients {
    class HumanPlayer : public Player {
        std::string name;
        void printGameState(const GameState* gs) const;
        bool askAndDiscard(GameState* gs);
        void askAndAdd(GameState* gs);
    public:
        explicit HumanPlayer(const std::string&  n) : name(n) {}

        bool runTurn(GameState* gs) override;
        [[nodiscard]] std::shared_ptr<Player> clone() const override;
    };
} // game

#endif //DUMMY_HUMANPLAYER_H