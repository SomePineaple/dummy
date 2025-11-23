//
// Created by nj60 on 11/17/25.
//

#include "nn_logic.h"

#include "../game/game.h"

namespace rummy::nn {
    NNLogic::NNLogic(const network<sequential>& e, const network<sequential>& n) : embedder(e), net(n) {
        if (embedder.in_data_size() != 17 || embedder.out_data_size() != CARD_EMBEDDING_SIZE) {
            throw std::runtime_error("Wrong embedder network size");
        }

        if (net.in_data_size() != NET_INPUT_SIZE || net.out_data_size() != NET_OUTPUT_SIZE) {
            throw std::runtime_error("Wrong main network dimensions");
        }
    }

    vec_t NNLogic::get_card_embedding(const Card& c) {
        const auto embedding_iterator = embeddings.find(c.get_sort_value());
        if (embedding_iterator == embeddings.end()) {
            auto embedded = embedder.predict(c.one_hot());
            embeddings[c.get_sort_value()] = embedded;
            return embedded;
        }

        return embedding_iterator->second;
    }

    void NNLogic::init_gs(const GameState *gs) {
        array<float, CARD_EMBEDDING_SIZE> padding{};
        padding.fill(0);

        vector<float> net_in;
        // Normalize to -1 -> 1 scale
        net_in.push_back(static_cast<float>(gs->stockPile.size()) / 12.5f - 1.0f);
        net_in.push_back(2.0f * static_cast<float>(gs->opponent->get_hand_size()) / MAX_HAND_SIZE - 1.0f);

        // Add discard pile
        for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
            if (i < gs->discardPile.size()) {
                vec_t embed_vec = get_card_embedding(*gs->discardPile.get_card(i));
                net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Add cards on the table
        for (int i = 0; i < MAX_PLAYED_CARDS; i++) {
            if (i < gs->melds.size()) {
                for (const auto& card : gs->melds[i]->get_cards()) {
                    vec_t embed_vec = get_card_embedding(*card);
                    net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
                }
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Add player hand
        for (int i = 0; i < MAX_HAND_SIZE; i++) {
            if (i < gs->player->get_hand_size()) {
                vec_t embed_vec = get_card_embedding(*gs->player->get_card(i));
                net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Finally run network
        net_output = net.predict(net_in);
    }

    uint8_t NNLogic::get_draw() const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        // We take the most probable prediction of the first 26 elements, where 0 is draw from stock, and any other is draw from discard at that index
        const auto largest_location = max_element(net_output.begin(), net_output.begin() + MAX_DISCARD_SIZE + 1);
        return largest_location - net_output.begin();
    }

    vector<uint8_t> NNLogic::get_play_cards() const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        vector<tuple<float, uint8_t>> card_outputs;
        for (int i = PLAY_OFFSET; i < DISCARD_OFFSET; i++) {
            if (net_output[i] > PLAY_ACTIVATION_FLOOR)
                card_outputs.emplace_back(net_output[i], i - PLAY_OFFSET);
        }

        partial_sort(card_outputs.begin(), card_outputs.begin() + 3, card_outputs.end(), [](const auto& a, const auto& b) {
            return get<0>(a) > get<0>(b);
        });

        vector<uint8_t> play_cards;
        for (int i = 0; i < min(card_outputs.size(), static_cast<size_t>(3)); i++) {
            play_cards.push_back(get<1>(card_outputs[i]));
        }

        return play_cards;
    }

    uint8_t NNLogic::get_discard() const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        // Location of discard output with the highest activation
        const auto largest_location = max_element(net_output.begin() + DISCARD_OFFSET, net_output.end());
        return largest_location - (net_output.begin() + DISCARD_OFFSET);
    }
} // rummy::clients
