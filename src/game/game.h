//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H
#include "player.h"

using namespace std;

namespace game {
    struct GameState {
        const std::shared_ptr<Player> opponent;
        const bool p1Turn;
        Pile stockPile;
        Pile discardPile;
        vector<shared_ptr<Meld>> melds;

        GameState(const shared_ptr<Player>& o, const bool p1Turn) :
            opponent(o), p1Turn(p1Turn), stockPile(getFullDeck()) {}
    };

    class Game {
        shared_ptr<Player> p1;
        shared_ptr<Player> p2;
        unique_ptr<GameState> gs;
    public:
        Game();
        bool isGameOver();
    };
} // game

#endif //DUMMY_GAME_H