//
// Created by nj60 on 11/17/25.
//

#include "nn_player.h"

#include "../game/game.h"


namespace rummy::nn {
    nn_logic::nn_logic(const network<sequential>& e, const network<sequential>& n) : embedder(e), net(n) {
        if (embedder.in_data_size() != 17 || embedder.out_data_size() != CARD_EMBEDDING_SIZE) {
            throw std::runtime_error("Wrong embedder network size");
        }

        if (net.in_data_size() != NET_INPUT_SIZE || net.out_data_size() != NET_OUTPUT_SIZE) {
            throw std::runtime_error("Wrong main network dimensions");
        }
    }


    vec_t nn_logic::get_card_embedding(const card& c) {
        auto embedding_iterator = embeddings.find(c.get_sort_value());
        if (embedding_iterator == embeddings.end()) {
            auto embedded = embedder.predict(c.one_hot());
            embeddings[c.get_sort_value()] = embedded;
            return embedded;
        }

        return embedding_iterator->second;
    }

    void nn_logic::init_gs(const game_state *gs) {
        std::array<float, CARD_EMBEDDING_SIZE> padding;
        padding.fill(0);

        std::vector<float> net_in;
        // Normalize to -1 -> 1 scale
        net_in.push_back(gs->stockPile.size() / 12.5 - 1.0);
        net_in.push_back(2 * gs->opponent->hand_size() / MAX_HAND_SIZE - 1.0);

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
                for (auto card : gs->melds[i]->get_cards()) {
                    vec_t embed_vec = get_card_embedding(*card);
                    net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
                }
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Add player hand
        for (int i = 0; i < MAX_HAND_SIZE; i++) {
            if (i < gs->player->hand_size()) {
                vec_t embed_vec = get_card_embedding(*gs->player->get_card(i));
                net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Finally run network
        net_output = net.predict(net_in);
    }
} // rummy::clients
