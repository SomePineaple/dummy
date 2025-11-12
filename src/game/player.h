//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H
#include "meld.h"

namespace game {
    struct GameState;

    class Player {
        Meld workingMeld;
        Pile hand;
        std::vector<Meld> playedMelds;
        std::string name;
        void printGameState(const GameState& gs) const;
        bool askAndDiscard();
        void askAndAdd();
    protected:
        void drawFromStock(GameState& gs);
        bool drawFromDiscard(GameState& gs, unsigned char numCards);
        bool playWorkingMeld(GameState& gs);
        void discard(GameState& gs, unsigned char cardNumber);
    public:
        Player(const std::string& name) : name(name){}
        unsigned short calcPoints();
        bool runTurn(GameState& gs);
    };
} // game

#endif //DUMMY_PLAYER_H
