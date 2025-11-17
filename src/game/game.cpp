//
// Created by nj60 on 11/12/25.
//

#include "game.h"

#include "humanplayer.h"
#include "player.h"
#include <memory>

namespace rummy {
    using namespace clients;

    GameState::GameState(const shared_ptr<Player>& o, const shared_ptr<Player>& p) :
            opponent(o), player(p), stockPile(getFullDeck()) {
        stockPile.shuffle();
    }

    GameState::GameState(const GameState* clone):
        opponent(clone->opponent->clone()),
        player(clone->player->clone()),
        stockPile(clone->stockPile),
        discardPile(clone->discardPile),
        melds(clone->melds) {}

    Game::Game() {
        p1 = make_shared<HumanPlayer>("Player 1");
        p2 = make_shared<HumanPlayer>("Player 2");

        gs = make_unique<GameState>(p2, p1);

        p1->drawFromStock(gs.get(), 7);
        p2->drawFromStock(gs.get(), 7);
    }

    Game::Game(const GameState& gs) {
        p1 = gs.player;
        p2 = gs.opponent;

        this->gs = make_unique<GameState>(gs);
    }

    Game::Game(const shared_ptr<Player> &p1, const shared_ptr<Player> &p2) {

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
