//
// Created by nj60 on 11/17/25.
//

#ifndef DUMMY_NNPLAYER_H
#define DUMMY_NNPLAYER_H

#include "../game/game.h"
#include "../game/meld.h"
#include <dlib/dnn.h>

namespace rummy::nn {
    using namespace dlib;

    constexpr size_t HIDDEN_LAYER_SIZE = 512;

    constexpr uint16_t CARD_EMBEDDING_SIZE = 16;
    // 0:draw stock, 1-25: Draw discard, 26-46: cards to play, 47-67: card to discard
    constexpr uint16_t NET_OUTPUT_SIZE = 68;
    constexpr uint16_t MAX_PLAYED_CARDS = 51;
    constexpr uint16_t MAX_DISCARD_SIZE = 25;
    constexpr uint16_t MAX_HAND_SIZE = 26;
    // 0: num cards in stock, 1: num cards in opponents hands, up to 25 cards in discard pile, up to MAX_PLAYED_CARDS played cards on the table, and then up to 26 cards in the current players hand
    constexpr uint16_t NET_INPUT_SIZE = 1 + 1 + MAX_DISCARD_SIZE*CARD_EMBEDDING_SIZE + MAX_PLAYED_CARDS*CARD_EMBEDDING_SIZE + MAX_HAND_SIZE*CARD_EMBEDDING_SIZE;

    using embedder_t = loss_metric<
            fc<CARD_EMBEDDING_SIZE,
            htan<fc<32,
            htan<fc<32,
            input<matrix<float, 1, 17
    >>>>>>>>;

    using actor_t = loss_metric<
        sig<fc<NET_OUTPUT_SIZE,
        htan<fc<HIDDEN_LAYER_SIZE,
        htan<fc<HIDDEN_LAYER_SIZE*2,
        input<matrix<float, 1, NET_INPUT_SIZE
    >>>>>>>>>;

    using embed_output_t = matrix<float, 1, CARD_EMBEDDING_SIZE>;

    using card_input_t = matrix<float, 1, 17>;

    using net_input_t = matrix<float, 1, NET_INPUT_SIZE>;

    class NNLogic {
        // Learning rate is 1 / sqrt(num parameters)
        const double LEARNING_RATE = 0.075;

        static constexpr uint16_t DISCARD_OFFSET = 47;
        static constexpr uint16_t PLAY_OFFSET = 26;
        static constexpr float PLAY_ACTIVATION_FLOOR = 0.5;

        shared_ptr<embedder_t> msp_embedder;
        shared_ptr<actor_t> msp_actor;
        // the key is the sort value of the cards.
        std::unordered_map<uint8_t, embed_output_t> embeddings{};
        matrix<float, 1, NET_OUTPUT_SIZE> net_output;

        float m_mutationRate;

        embed_output_t get_card_embedding(const Card& c);
    public:
        explicit NNLogic(float mutationRate);
        NNLogic(const shared_ptr<embedder_t>& e, const shared_ptr<actor_t>& n);
        NNLogic(const NNLogic& mutateFrom, float mutationChance);
        NNLogic(const NNLogic& from);
        NNLogic(const string& loadPath, float mutationRate);
        void init_gs(const GameState* gs);

        // returns 0 to draw from stock, and anything more is how many to draw from discard.
        uint8_t get_draw(const std::vector<bool>& discardMask) const;
        std::vector<uint8_t> get_play_cards(const std::vector<bool>& playMask) const;
        uint8_t get_discard(uint16_t handSize) const;
        void write_to_file(const string& prefix) const;
    };
} // rummy::nn

#endif //DUMMY_NNPLAYER_H
