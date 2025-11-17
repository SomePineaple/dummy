//
// Created by nj60 on 11/12/25.
//

#include "game.h"

#include "player.h"
#include <memory>

namespace rummy {

    game_state::game_state(const shared_ptr<clients::player>& p, const shared_ptr<clients::player>& o) :
            opponent(o), player(p), stockPile(getFullDeck()) {
        stockPile.shuffle();
        player->drawFromStock(this, 13);
        opponent->drawFromStock(this, 13);
        stockPile.dump(discardPile, 1);
    }

    game_state::game_state(const game_state* clone):
        opponent(clone->opponent->clone()),
        player(clone->player->clone()),
        stockPile(clone->stockPile),
        discardPile(clone->discardPile),
        melds(clone->melds) {}

    game::game(const game_state& gs) {
        p1 = gs.player;
        p2 = gs.opponent;

        this->gs = make_unique<game_state>(gs);
    }

    game::game(const shared_ptr<clients::player>& p1, const shared_ptr<clients::player>& p2) {
        gs = make_unique<game_state>(p1, p2);
        this->p1 = p1;
        this->p2 = p2;
    }


    game_winner game::get_winner() const {
        if (p1->hand_size() == 0 || p2->hand_size() == 0 || gs->stockPile.size() == 0) {
            return p1->calcPoints() > p2->calcPoints() ? PLAYER_ONE : PLAYER_TWO;
        }

        return NOT_OVER;
    }

    void game::run_round() {
        auto backupState = make_unique<game_state>(gs.get());

        if (!gs->player->run_turn(gs.get())) {
            gs.swap(backupState);
        } else {
            swap(gs->player, gs->opponent);
        }
    }
} // game
