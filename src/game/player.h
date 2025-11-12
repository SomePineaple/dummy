//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H

#include "meld.h"

namespace game {
    struct GameState;
}

namespace game::clients {
    class Player {
    protected:
        Meld workingMeld;
        Pile hand;
        std::vector<std::shared_ptr<Meld>> playedMelds;

        bool drawFromDiscard(GameState* gs, unsigned char numCards);
        bool playWorkingMeld(GameState* gs);
        bool discard(GameState* gs, unsigned char cardNumber);
        bool addToWorkingMeld(unsigned char cardNumber);
    public:
        virtual ~Player() = default;

        bool drawFromStock(GameState* gs, unsigned char numCards);
        unsigned short calcPoints();
        void printMelds() const;
        [[nodiscard]] unsigned char getHandSize() const;
        virtual bool runTurn(GameState* gs);
        [[nodiscard]] virtual std::shared_ptr<Player> clone() const;
    };
} // game

#endif //DUMMY_PLAYER_H
