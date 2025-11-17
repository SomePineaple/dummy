//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H
#include "player.h"

using namespace std;

namespace rummy {
    enum game_winner {
        PLAYER_ONE, PLAYER_TWO, NOT_OVER
    };

    struct game_state {
        std::shared_ptr<clients::player> opponent;
        shared_ptr<clients::player> player;
        pile stockPile;
        pile discardPile;
        vector<shared_ptr<meld>> melds;

        game_state(const shared_ptr<clients::player>& p, const shared_ptr<clients::player>& o);

        explicit game_state(const game_state* clone);
    };

    class game {
        shared_ptr<clients::player> p1;
        shared_ptr<clients::player> p2;
        unique_ptr<game_state> gs;
    public:
        explicit game(const game_state& gs);
        game(const shared_ptr<clients::player>& p1, const shared_ptr<clients::player>& p2);
        void run_round();
        [[nodiscard]] game_winner get_winner() const;
    };
} // game

#endif //DUMMY_GAME_H