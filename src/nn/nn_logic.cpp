//
// Created by nj60 on 11/17/25.
//

#include "nn_logic.h"

#include "../game/game.h"
#include "dlib_helper.h"
#include <random>

namespace rummy::nn {
    // If no parameters are passed in, we initialize everything with random distribution
    NNLogic::NNLogic() {
        msp_embedder = make_shared<embedder_t>();
        msp_actor = make_shared<actor_t>();

        nn_helper::initialize_network_gaussian(*msp_embedder, 0.0f, 2.0f);
        nn_helper::initialize_network_gaussian(*msp_actor, 0.0f, 2.0f);
    }

    NNLogic::NNLogic(const NNLogic& from) {
        // Deep copy the networks
        msp_embedder = make_shared<embedder_t>(*from.msp_embedder);
        msp_actor = make_shared<actor_t>(*from.msp_actor);
    }

    NNLogic::NNLogic(const NNLogic& mutateFrom, const float mutationStrength, const float mutationChance) {
        msp_embedder = make_shared<embedder_t>(*mutateFrom.msp_embedder);
        msp_actor = make_shared<actor_t>(*mutateFrom.msp_actor);

        nn_helper::mutate_network(*msp_embedder, mutationStrength, mutationChance);
        nn_helper::mutate_network(*msp_actor, mutationStrength, mutationChance);
    }

    NNLogic::NNLogic(const shared_ptr<embedder_t>& e, const shared_ptr<actor_t>& n) : msp_embedder(e), msp_actor(n) {

    }

    embed_output_t NNLogic::get_card_embedding(const Card& c) {
        const int key = c.get_sort_value();
        const auto it = embeddings.find(key);
        if (it == embeddings.end()) {
            const auto x = c.one_hot();

            // Forward pass through the net
            embed_output_t embedded = trans((*msp_embedder)(x));

            embeddings.emplace(key, embedded);
            return embedded;
        }

        return it->second;
    }

    void NNLogic::init_gs(const GameState *gs) {
        embed_output_t padding{};
        padding = 0;

        std::vector<float> net_in;
        // Normalize to -1 -> 1 scale
        net_in.push_back(static_cast<float>(gs->stockPile.size()) / 12.5f - 1.0f);
        net_in.push_back(2.0f * static_cast<float>(gs->opponent->get_hand_size()) / MAX_HAND_SIZE - 1.0f);

        // Add discard pile
        for (int i = 0; i < MAX_DISCARD_SIZE; i++) {
            if (i < gs->discardPile.size()) {
                embed_output_t embed_vec = get_card_embedding(*gs->discardPile.get_card(i));
                net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Add cards on the table
        for (int i = 0; i < MAX_PLAYED_CARDS; i++) {
            if (i < gs->melds.size()) {
                for (const auto& card : gs->melds[i]->get_cards()) {
                    embed_output_t embed_vec = get_card_embedding(*card);
                    net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
                }
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        // Add player hand
        for (int i = 0; i < MAX_HAND_SIZE; i++) {
            if (i < gs->player->get_hand_size()) {
                embed_output_t embed_vec = get_card_embedding(*gs->player->get_card(i));
                net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
            } else {
                padding(0);
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        net_input_t x;
        for (int i = 0; i < net_in.size(); i++) {
            x(0, i) = net_in[i];
        }

        // Finally run network
        net_output = trans((*msp_actor)(x));
    }

    uint8_t NNLogic::get_draw() const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        // We take the most probable prediction of the first 26 elements, where 0 is draw from stock, and any other is draw from discard at that index
        const auto largest_location = max_element(net_output.begin(), net_output.begin() + MAX_DISCARD_SIZE + 1);
        return largest_location - net_output.begin();
    }

    std::vector<uint8_t> NNLogic::get_play_cards() const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        std::vector<tuple<float, uint8_t>> card_outputs;
        for (int i = PLAY_OFFSET; i < DISCARD_OFFSET; i++) {
            if (net_output(0, i) > PLAY_ACTIVATION_FLOOR)
                card_outputs.emplace_back(net_output(0, i), i - PLAY_OFFSET);
        }

        if (!card_outputs.empty()) {
            partial_sort(card_outputs.begin(), min(card_outputs.begin() + 3, card_outputs.end()), card_outputs.end(), [](const auto& a, const auto& b) {
                return get<0>(a) > get<0>(b);
            });
        }

        std::vector<uint8_t> play_cards;
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

    void NNLogic::write_to_file(const string& prefix) const {
        serialize(prefix + "_embedder.bin") << (*msp_embedder);
        serialize(prefix + "_actor.bin") << (*msp_actor);
    }

} // rummy::clients
