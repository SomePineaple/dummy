//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H

#include "meld.h"
#include <string>

namespace rummy {
    struct game_state;
}

namespace rummy::clients {
    class player {
    protected:
        meld workingMeld;
        pile hand;
        std::vector<std::shared_ptr<meld>> playedMelds;

        bool draw_from_discard(game_state* gs, unsigned char numCards);
        bool play_working_meld(game_state* gs);
        bool discard(game_state* gs, unsigned char cardNumber);
        bool add_to_working_meld(unsigned char cardNumber);
    public:
        virtual ~player() = default;

        bool draw_from_stock(game_state* gs, unsigned char numCards);
        [[nodiscard]] std::shared_ptr<card> get_card(uint8_t index) const;
        [[nodiscard]] int calc_points() const;
        [[nodiscard]] std::string print_melds() const;
        [[nodiscard]] unsigned char hand_size() const;
        virtual bool run_turn(game_state* gs);
        [[nodiscard]] virtual std::shared_ptr<player> clone() const;
        virtual void clean();
    };
} // game

#endif //DUMMY_PLAYER_H
