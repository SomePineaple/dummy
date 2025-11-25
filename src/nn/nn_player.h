//
// Created by nj60 on 11/18/25.
//

#ifndef DUMMY_NN_PLAYER_H
#define DUMMY_NN_PLAYER_H

#include "nn_logic.h"
#include "../game/player.h"

namespace rummy::nn {
    class NNPlayer final : public clients::Player {
        shared_ptr<NNLogic> msp_logic;
        std::vector<Meld> m_ToPlay;
        std::vector<std::vector<uint8_t>> m_PlayableMelds;

        // Stores the playable melds and the playable cards mask for any given card in the discard pile.
        std::unordered_map<uint8_t, tuple<std::vector<std::vector<uint8_t>>, std::vector<bool>>> m_PlayableMeldsWithDiscard;

        void try_play_cards(const std::vector<uint8_t>& cards);
        void try_play_cards(const std::vector<uint8_t>& cards, const shared_ptr<Card>& fromDiscard);
        void add_to_working_meld(const shared_ptr<Card>& card);
        std::vector<bool> get_hand_play_mask(const GameState* gs);
        std::vector<bool> get_discard_pile_mask(const GameState* gs);
        bool can_draw_discard(const shared_ptr<Card>& card);
    public:
        //NNPlayer(const network<sequential>& e, const network<sequential>& n) : msp_logic(make_shared<NNLogic>(e, n)), mopt_ToPlay(nullopt) {}
        explicit NNPlayer(const shared_ptr<NNLogic>& logic) : msp_logic(logic) {}

        bool run_turn(GameState* gs) override;
        // Draw from stock and discard a random card
        void random_turn(GameState* gs);

        uint16_t get_unplayed_points() const;

        [[nodiscard]] shared_ptr<Player> clone() const override;
    };
} // rummy::nn

#endif //DUMMY_NN_PLAYER_H
