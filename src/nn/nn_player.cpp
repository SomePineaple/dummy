//
// Created by nj60 on 11/17/25.
//

#include "nn_player.h"


namespace rummy::nn {
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

    }
} // rummy::clients
