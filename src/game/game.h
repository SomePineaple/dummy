//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_GAME_H
#define DUMMY_GAME_H

#include "meld.h"
#include "clients/player.h"

namespace rummy {
    enum class GameStatus {
        NOT_OVER, P1_WINS, P2_WINS
    };

    struct GameState {
        std::shared_ptr<clients::Player> opponent;
        std::shared_ptr<clients::Player> player;
        Pile stockPile;
        Pile discardPile;
        std::vector<std::shared_ptr<Meld>> melds;

        GameState(const std::shared_ptr<clients::Player>& p, const std::shared_ptr<clients::Player>& o);

        explicit GameState(const GameState* clone);

        [[nodiscard]] uint8_t get_num_cards() const;
    };

    class Game {
        std::shared_ptr<clients::Player> m_p1;
        std::shared_ptr<clients::Player> m_p2;
        std::unique_ptr<GameState> m_gs;
    public:
        explicit Game(const GameState& gs);
        Game(const std::shared_ptr<clients::Player>& p1, const std::shared_ptr<clients::Player>& p2);
        void run_round();
        [[nodiscard]] GameStatus is_game_over() const;
        void notify_players() const;
    };
} // game

#endif //DUMMY_GAME_H