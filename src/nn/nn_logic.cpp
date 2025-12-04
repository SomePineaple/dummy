//
// Created by nj60 on 11/17/25.
//

#include "nn_logic.h"

#include "../game/game.h"
#include "dlib_helper.h"
#include <random>

namespace rummy::nn {
    NNLogic::NNLogic(const string& loadPath, const float mutationRate) {
        m_mutationRate = mutationRate;
        msp_embedder = make_shared<embedder_t>();
        msp_actor = make_shared<actor_t>();

        deserialize(loadPath + "_embedder.bin") >> (*msp_embedder);
        deserialize(loadPath + "_actor.bin") >> (*msp_actor);
    }

    // If no parameters are passed in, we initialize everything with random distribution
    NNLogic::NNLogic(const float mutationRate) {
        msp_embedder = make_shared<embedder_t>();
        msp_actor = make_shared<actor_t>();
        m_mutationRate = mutationRate;

        // Ensure we have initialized weights
        net_input_t x;
        x = 0;
        (*msp_actor)(x);

        matrix<float, 1, 17> f;
        f = 0;
        (*msp_embedder)(f);
    }

    NNLogic::NNLogic(const NNLogic& from) {
        m_mutationRate = from.m_mutationRate;

        // Deep copy the networks
        msp_embedder = make_shared<embedder_t>(*from.msp_embedder);
        msp_actor = make_shared<actor_t>(*from.msp_actor);
    }

    NNLogic::NNLogic(const NNLogic& mutateFrom, const float mutationChance) {
        dlib::rand rnd;

        // Use σ′=σ⋅exp(τz) to update the mutation rate.
        m_mutationRate = static_cast<float>(clamp(mutateFrom.m_mutationRate * exp(LEARNING_RATE * rnd.get_random_gaussian()), 0.001, 0.1));

        msp_embedder = make_shared<embedder_t>(*mutateFrom.msp_embedder);
        msp_actor = make_shared<actor_t>(*mutateFrom.msp_actor);

        // Smaller mutation rate on the embedder so the actor doesn't go crazy because we change its input
        nn_helper::mutate_network(msp_embedder, m_mutationRate * 0.1f, mutationChance);
        nn_helper::mutate_network(msp_actor, m_mutationRate, mutationChance);
    }

    NNLogic::NNLogic(const shared_ptr<embedder_t>& e, const shared_ptr<actor_t>& n) : msp_embedder(e), msp_actor(n) {
        m_mutationRate = 0.01;
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
        int cardsAdded = 0;
        for (int i = 0; i < gs->melds.size(); i++) {
            if (i < gs->melds.size()) {
                for (const auto& card : gs->melds[i]->get_cards()) {
                    if (cardsAdded >= MAX_PLAYED_CARDS)
                        break;

                    embed_output_t embed_vec = get_card_embedding(*card);
                    net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
                    cardsAdded++;
                }
            }
        }
        for (int i = cardsAdded; i < MAX_PLAYED_CARDS; i++) {
            net_in.insert(net_in.end(), padding.begin(), padding.end());
        }

        // Add player hand
        for (int i = 0; i < MAX_HAND_SIZE; i++) {
            if (i < gs->player->get_hand_size()) {
                embed_output_t embed_vec = get_card_embedding(*gs->player->get_card(i));
                net_in.insert(net_in.end(), embed_vec.begin(), embed_vec.end());
            } else {
                net_in.insert(net_in.end(), padding.begin(), padding.end());
            }
        }

        net_input_t x;
        for (int i = 0; i < NET_INPUT_SIZE; i++) {
            x(0, i) = net_in[i];
        }

        // Finally run network
        net_output = trans((*msp_actor)(x));
    }

    uint8_t NNLogic::get_draw(const std::vector<bool>& discardMask) const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        float max = 0;
        uint8_t maxIdx = 0;

        for (int i = 0; i <= discardMask.size(); i++) {
            if (net_output(0, i) > max && discardMask[i - 1]) {
                maxIdx = i;
                max = net_output(0, i);
            }
        }

        return maxIdx;
    }

    std::vector<uint8_t> NNLogic::get_play_cards(const std::vector<bool>& playMask) const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        std::vector<uint8_t> card_outputs;
        for (uint16_t i = PLAY_OFFSET; i < PLAY_OFFSET + playMask.size(); i++) {
            if (net_output(0, i) > PLAY_ACTIVATION_FLOOR && playMask[i - PLAY_OFFSET])
                card_outputs.emplace_back(i - PLAY_OFFSET);
        }

        return card_outputs;
    }

    uint8_t NNLogic::get_discard(const uint16_t handSize) const {
        if (net_output.size() != NET_OUTPUT_SIZE) {
            throw runtime_error("Network output has not been initialized");
        }

        // Location of discard output with the highest activation
        const auto largest_location = max_element(net_output.begin() + DISCARD_OFFSET, net_output.begin() + min(static_cast<int>(NET_OUTPUT_SIZE), DISCARD_OFFSET + handSize));
        return largest_location - (net_output.begin() + DISCARD_OFFSET);
    }

    void NNLogic::write_to_file(const string& prefix) const {
        msp_embedder->clean();
        msp_actor->clean();

        serialize(prefix + "_embedder.bin") << (*msp_embedder);
        serialize(prefix + "_actor.bin") << (*msp_actor);
    }

} // rummy::clients
