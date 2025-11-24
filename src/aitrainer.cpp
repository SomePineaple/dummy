//
// Created by nj60 on 11/17/25.
//

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>

#include "nn/nn_logic.h"
#include "nn/nn_player.h"

using namespace std;
namespace rn = rummy::nn;
using Logic = shared_ptr<rn::NNLogic>;

constexpr int GENERATION_SIZE = 100;
constexpr int BAD_MOVE_REWARD = -2;

void create_init_generation(vector<Logic>& generation, const float mutationStrength) {
    vector<thread> threads;
    mutex generationLock;

    for (int i = 0; i < GENERATION_SIZE; i++) {
        threads.emplace_back([&generation, &generationLock, mutationStrength] {
            // Begin with a mutation rate of 0.01
            const auto l = make_shared<rn::NNLogic>(mutationStrength);
            lock_guard lock(generationLock);
            generation.push_back(l);
        });
    }

    for_each(threads.begin(), threads.end(), [](auto& thread) {thread.join();});
}

int test_networks(const Logic& a, const Logic& b) {
    int score = 0;

    auto p1 = make_shared<rn::NNPlayer>(a);
    auto p2 = make_shared<rn::NNPlayer>(b);

    auto gs = std::make_unique<rummy::GameState>(p1, p2);

    bool p1IsPlayer = true;

    do {
        auto backupGs = std::make_unique<rummy::GameState>(gs.get());
        if (!gs->player->run_turn(gs.get())) {
            swap(gs, backupGs);
            static_pointer_cast<rn::NNPlayer>(gs->player)->random_turn(gs.get());
            score += BAD_MOVE_REWARD;
        }
        swap(gs->player, gs->opponent);
        p1IsPlayer = !p1IsPlayer;
    } while (gs->player->get_hand_size() > 0 && p2->get_hand_size() > 0 && gs->stockPile.size() > 0);

    return score + ((p1IsPlayer ? 1 : -1) * (gs->player->calc_points() - gs->opponent->calc_points()));
}

void test_generation(const vector<Logic>& generation, vector<tuple<Logic, int>>& scoring) {
    mutex scoringMutex;
    vector<thread> threads;

    // Test every network against every other network twice
    for (int i = 0; i < generation.size(); i++) {
        threads.emplace_back([&scoringMutex, &scoring, &generation, i] {
            int score = 0;
            // Clone NNLogic to avoid memory races.
            const auto a = make_shared<rn::NNLogic>(*generation[i]);
            for (int f = i; f < i +generation.size(); f++) {
                const uint64_t j = f % generation.size();
                if (i == j) continue;

                // Clone the NNLogic to avoid memory races.
                auto b = make_shared<rn::NNLogic>(*generation[j]);

                score += test_networks(a, b);
            }

            lock_guard scoringLock(scoringMutex);
            scoring.emplace_back(generation[i], score);
        });
    }

    for_each(threads.begin(), threads.end(), [](auto& thread) {thread.join();});
}

void sim_generations(const uint16_t numGenerations, const uint16_t keepTop, const uint16_t introduceNew, const float mutationChance, const float mutationStrength) {
    vector<Logic> networks;
    cout << "Creating initial generation..." << endl;
    create_init_generation(networks, mutationStrength);
    cout << "done" << endl;
    for (int i = 0; i < numGenerations; i++) {
        vector<tuple<Logic, int>> scoring;
        cout << "Testing generation " << i << "..." << endl;
        test_generation(networks, scoring);
        cout << "Done testing" << endl;
        partial_sort(scoring.begin(), scoring.begin() + keepTop, scoring.end(), [](const auto& a, const auto& b) {
            return get<1>(a) > get<1>(b);
        });

        // Now we evolve
        networks.clear();
        for (int j = 0; j < keepTop; j++) {
            get<0>(scoring[j])->write_to_file("net_" + to_string(j));
            const uint16_t numChildren = (GENERATION_SIZE - introduceNew) / keepTop - 1;
            cout << "Now evolving net number " << j << " who scored " << get<1>(scoring[j]) << endl;
            for (int k = 0; k < numChildren; k++) {
                networks.push_back(make_shared<rummy::nn::NNLogic>(*get<0>(scoring[j]), mutationChance));
            }

            networks.push_back(get<0>(scoring[j]));
        }

        // Add some new completely random nets
        for (uint64_t j = networks.size(); j < GENERATION_SIZE; j++) {
            networks.push_back(make_shared<rn::NNLogic>(mutationStrength));
        }
    }
}

int main() {
    sim_generations(20, 10, 10, 1, 0.01);
    return 0;
}
