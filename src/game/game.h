//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H
#include "player.h"

using namespace std;

namespace rummy {
    struct GameState {
        std::shared_ptr<clients::Player> opponent;
        shared_ptr<clients::Player> player;
        Pile stockPile;
        Pile discardPile;
        vector<shared_ptr<Meld>> melds;

        GameState(const shared_ptr<clients::Player>& p, const shared_ptr<clients::Player>& o);

        explicit GameState(const GameState* clone);
    };

    class Game {
        shared_ptr<clients::Player> p1;
        shared_ptr<clients::Player> p2;
        unique_ptr<GameState> gs;
    public:
        explicit Game(const GameState& gs);
        Game(const shared_ptr<clients::Player>& p1, const shared_ptr<clients::Player>& p2);
        void runRound();
        [[nodiscard]] bool isGameOver() const;
    };
} // game

#endif //DUMMY_GAME_H