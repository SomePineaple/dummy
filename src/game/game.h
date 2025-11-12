//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H
#include "player.h"

using namespace std;

namespace game {
    struct GameState {
        std::shared_ptr<Player> opponent;
        shared_ptr<Player> player;
        Pile stockPile;
        Pile discardPile;
        vector<shared_ptr<Meld>> melds;

        GameState(const shared_ptr<Player>& o, const shared_ptr<Player>& p);

        explicit GameState(const GameState* clone);
    };

    class Game {
        shared_ptr<Player> p1;
        shared_ptr<Player> p2;
        unique_ptr<GameState> gs;
    public:
        Game();
        void runRound();
        [[nodiscard]] bool isGameOver() const;
    };
} // game

#endif //DUMMY_GAME_H