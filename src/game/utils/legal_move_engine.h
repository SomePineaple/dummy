//
// Created by nj60 on 11/26/25.
//

#ifndef DUMMY_LEGAL_MOVE_ENGINE_H
#define DUMMY_LEGAL_MOVE_ENGINE_H

#include <vector>
#include <unordered_map>
#include "../game.h"

namespace rummy::utils {
    class LegalMoveEngine final {
        // Stores the playable melds and the playable cards mask for any given card in the discard pile, (and with drawing from stock)
        std::unordered_map<uint8_t, std::tuple<std::vector<std::vector<uint8_t>>, std::vector<bool>>> m_PlayableMeldsWithDraw;
        std::vector<bool> m_DiscardMask;

        void update_no_draw(const GameState* gs, const Pile& hand);
        void update_discard(const GameState* gs, const Pile& hand);
    public:
        LegalMoveEngine(const GameState* gs, const Pile& hand);
        std::vector<bool> get_hand_play_mask(uint8_t withDraw = 0);
        std::vector<std::vector<uint8_t>> get_playable_melds(uint8_t withDraw = 0);
        std::vector<bool> get_discard_pile_mask() const;
    };
} // rummy::utils

#endif //DUMMY_LEGAL_MOVE_ENGINE_H