//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H
#include "player.h"

using namespace std;

namespace game {
    struct GameState {
        const Player* opponent;
        const bool p1Turn;
        Pile stockPile;
        Pile discardPile;
        vector<shared_ptr<Meld>> melds;

        GameState(const Player* o, const bool p1Turn) :
            opponent(o), p1Turn(p1Turn), stockPile(getFullDeck()) {}
    };

    class Game {
        Player* p1;
        Player* p2;
        GameState* gs;
    public:
        Game();
        ~Game();
        bool isGameOver();
    };
} // game

#endif //DUMMY_GAME_H