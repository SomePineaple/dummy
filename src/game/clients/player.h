//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H

#include "../meld.h"
#include <string>
#include <memory>

namespace rummy {
    struct GameState;
}

namespace rummy::clients {
    class Player {
    protected:
        Meld m_WorkingMeld;
        Pile m_hand;
        std::vector<std::shared_ptr<Meld>> m_PlayedMelds;

        bool draw_from_discard(GameState* gs, uint8_t numCards);
        bool play_working_meld(GameState* gs);
        bool discard(GameState* gs, uint8_t cardNumber);
        bool add_to_working_meld(uint8_t cardNumber);
    public:
        virtual ~Player() = default;

        bool draw_from_stock(GameState* gs, uint8_t numCards);
        int16_t calc_points();
        [[nodiscard]] string print_melds() const;
        [[nodiscard]] uint8_t get_hand_size() const;
        [[nodiscard]] std::shared_ptr<Card> get_card(uint8_t index) const;

        // Returns false if the player made an illegal move and we need to reset.
        virtual bool run_turn(GameState* gs);

        [[nodiscard]] virtual std::shared_ptr<Player> clone() const;
        virtual void close();
        virtual void notify_player(int16_t opponentPoints) {}
    };
} // game

#endif //DUMMY_PLAYER_H
