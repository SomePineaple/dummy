//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H

#include "meld.h"
#include "clients/player.h"

namespace rummy {
    using namespace std;

    enum GameStatus {
        NOT_OVER, P1_WINS, P2_WINS
    };

    struct GameState {
        shared_ptr<clients::Player> opponent;
        shared_ptr<clients::Player> player;
        Pile stockPile;
        Pile discardPile;
        vector<shared_ptr<Meld>> melds;

        GameState(const shared_ptr<clients::Player>& p, const shared_ptr<clients::Player>& o);

        explicit GameState(const GameState* clone);

        [[nodiscard]] uint8_t get_num_cards() const;
    };

    class Game {
        shared_ptr<clients::Player> m_p1;
        shared_ptr<clients::Player> m_p2;
        unique_ptr<GameState> m_gs;
    public:
        explicit Game(const GameState& gs);
        Game(const shared_ptr<clients::Player>& p1, const shared_ptr<clients::Player>& p2);
        void run_round();
        [[nodiscard]] GameStatus is_game_over() const;
        void notify_players() const;
    };
} // game

#endif //DUMMY_GAME_H