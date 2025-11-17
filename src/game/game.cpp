//
// Created by nj60 on 11/12/25.
//

#include "game.h"

#include "player.h"
#include <memory>

namespace rummy {
    using namespace clients;

    GameState::GameState(const shared_ptr<Player>& p, const shared_ptr<Player>& o) :
            opponent(o), player(p), stockPile(getFullDeck()) {
        stockPile.shuffle();
        player->drawFromStock(this, 13);
        opponent->drawFromStock(this, 13);
        stockPile.dump(discardPile, 1);
    }

    GameState::GameState(const GameState* clone):
        opponent(clone->opponent->clone()),
        player(clone->player->clone()),
        stockPile(clone->stockPile),
        discardPile(clone->discardPile),
        melds(clone->melds) {}

    Game::Game(const GameState& gs) {
        p1 = gs.player;
        p2 = gs.opponent;

        this->gs = make_unique<GameState>(gs);
    }

    Game::Game(const shared_ptr<Player>& p1, const shared_ptr<Player>& p2) {
        gs = make_unique<GameState>(p1, p2);
        this->p1 = p1;
        this->p2 = p2;
    }


    bool Game::isGameOver() const {
        return p1->getHandSize() == 0 || p2->getHandSize() == 0 || gs->stockPile.size() == 0;
    }

    void Game::runRound() {
        auto backupState = make_unique<GameState>(gs.get());

        if (!gs->player->runTurn(gs.get())) {
            gs.swap(backupState);
        } else {
            swap(gs->player, gs->opponent);
        }
    }
} // game
