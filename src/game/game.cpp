//
// Created by nj60 on 11/12/25.
//

#include "game.h"

#include "clients/player.h"
#include <memory>

namespace rummy {
    using namespace clients;

    GameState::GameState(const shared_ptr<Player>& p, const shared_ptr<Player>& o) :
            opponent(o), player(p), stockPile(get_full_deck()) {
        stockPile.shuffle();
        player->draw_from_stock(this, 13);
        opponent->draw_from_stock(this, 13);
        stockPile.dump(discardPile, 1);
    }

    GameState::GameState(const GameState* clone):
        opponent(clone->opponent->clone()),
        player(clone->player->clone()),
        stockPile(clone->stockPile),
        discardPile(clone->discardPile),
        melds(clone->melds) {}

    Game::Game(const GameState& gs) {
        m_p1 = gs.player;
        m_p2 = gs.opponent;

        this->m_gs = make_unique<GameState>(gs);
    }

    Game::Game(const shared_ptr<Player>& p1, const shared_ptr<Player>& p2) {
        m_gs = make_unique<GameState>(p1, p2);
        this->m_p1 = p1;
        this->m_p2 = p2;
    }


    GameStatus Game::is_game_over() const {
        if (m_p1->get_hand_size() == 0 || m_p2->get_hand_size() == 0 || m_gs->stockPile.size() == 0) {
            return m_p1->calc_points() > m_p2->calc_points() ? P1_WINS : P2_WINS;
        }

        return NOT_OVER;
    }

    void Game::run_round() {
        auto backupState = make_unique<GameState>(m_gs.get());

        if (!m_gs->player->run_turn(m_gs.get())) {
            m_gs.swap(backupState);
        } else {
            swap(m_gs->player, m_gs->opponent);
        }
    }

    void Game::notify_players() const {
        m_p1->notify_player(m_p2->calc_points());
        m_p2->notify_player(m_p1->calc_points());
    }

} // game
