//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H
#include "player.h"

using namespace std;

namespace game {
    struct GameState {
        const Player& opponent;
        const bool p1Turn;
        const Pile stockPile;
        const Pile discardPile;
        const vector<shared_ptr<Meld>> melds;
    };

    class Game {
        Player* p1;
        Player* p2;
    public:
        Game();
        ~Game();
        bool isGameOver();
    };
} // game

#endif //DUMMY_GAME_H