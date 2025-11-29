//
// Created by nj60 on 11/26/25.
//

#ifndef DUMMY_RULE_BOT_H
#define DUMMY_RULE_BOT_H
#include "player.h"
#include "../utils/legal_move_engine.h"

namespace rummy::clients {
    class RuleBot final : public Player {
        std::vector<Meld> m_ToPlay;

        void try_play_cards(const std::vector<uint8_t>& cards);
        void try_play_cards(const std::vector<uint8_t>& cards, const shared_ptr<Card>& fromDiscard, utils::LegalMoveEngine& moveEngine);
        void add_to_working_meld(const shared_ptr<Card>& card);
    public:
        bool run_turn(GameState *gs) override;
        std::shared_ptr<Player> clone() const override;
    };
} // rummy::clients

#endif //DUMMY_RULE_BOT_H
