//
// Created by nj60 on 11/17/25.
//

#ifndef DUMMY_NNPLAYER_H
#define DUMMY_NNPLAYER_H

#include "../game/game.h"
#include <tiny_dnn/tiny_dnn.h>

namespace rummy::nn {
    using namespace tiny_dnn;

    constexpr uint16_t CARD_EMBEDDING_SIZE = 16;
    // 0:draw stock, 1-25: Draw discard, 26-46: cards to play, 47-67: card to discard
    constexpr uint16_t NET_OUTPUT_SIZE = 68;
    constexpr uint16_t MAX_PLAYED_CARDS = 51;
    constexpr uint16_t MAX_DISCARD_SIZE = 25;
    constexpr uint16_t MAX_HAND_SIZE = 26;
    // 0: num cards in stock, 1: num cards in opponents hands, up to 25 cards in discard pile, up to MAX_PLAYED_CARDS played cards on the table, and then up to 26 cards in the current players hand
    constexpr uint16_t NET_INPUT_SIZE = 1 + 1 + MAX_DISCARD_SIZE*CARD_EMBEDDING_SIZE + MAX_PLAYED_CARDS*CARD_EMBEDDING_SIZE + MAX_HAND_SIZE*CARD_EMBEDDING_SIZE;

    class NNLogic {
        static constexpr uint16_t DISCARD_OFFSET = 47;
        static constexpr uint16_t PLAY_OFFSET = 26;
        static constexpr float PLAY_ACTIVATION_FLOOR = 0.7;

        network<sequential> embedder;
        network<sequential> net;
        // the key is going to be the sort_value of the cards.
        std::unordered_map<uint8_t, vec_t> embeddings{};
        vec_t net_output;

        vec_t get_card_embedding(const Card& c);
    public:
        NNLogic(const network<sequential>& e, const network<sequential>& n);
        void init_gs(const GameState* gs);
        // returns 0 to draw from stock, and anything more is how many to draw from discard.
        uint8_t get_draw() const;
        std::vector<uint8_t> get_play_cards() const;
        uint8_t get_discard() const;
    };
} // rummy::nn

#endif //DUMMY_NNPLAYER_H
