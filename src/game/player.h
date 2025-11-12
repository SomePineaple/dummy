//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H
#include "meld.h"

namespace game {
    class Player {
        Meld workingMeld;
        void printGameState();
    public:
        int calcPoints();
        void runTurn();
    };
} // game

#endif //DUMMY_PLAYER_H
