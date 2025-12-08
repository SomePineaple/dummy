//
// Created by nj60 on 11/18/25.
//

#ifndef DUMMY_NN_PLAYER_H
#define DUMMY_NN_PLAYER_H

#include "nn_logic.h"
#include "../game/clients/player.h"
#include "../game/utils/legal_move_engine.h"

namespace rummy::nn {
    class NNPlayer final : public clients::Player {
        std::shared_ptr<NNLogic> msp_logic;
        std::vector<Meld> m_ToPlay;

        void try_play_cards(const std::vector<uint8_t>& cards, utils::LegalMoveEngine& moveEngine);
        void try_play_cards(const std::vector<uint8_t>& cards, const Card& fromDiscard, utils::LegalMoveEngine& moveEngine);
        void add_to_working_meld(const Card& card);
    public:
        //NNPlayer(const network<sequential>& e, const network<sequential>& n) : msp_logic(make_shared<NNLogic>(e, n)), mopt_ToPlay(nullopt) {}
        explicit NNPlayer(const std::shared_ptr<NNLogic>& logic) : msp_logic(logic) {}

        bool run_turn(GameState& gs) override;
        // Draw from stock and discard a random card
        void random_turn(GameState& gs);

        [[nodiscard]] uint16_t get_unplayed_points() const;

        [[nodiscard]] std::shared_ptr<Player> clone() const override;
    };
} // rummy::nn

#endif //DUMMY_NN_PLAYER_H
