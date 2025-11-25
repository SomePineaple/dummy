//
// Created by nj60 on 11/17/25.
//

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <dlib/rand.h>

#include "nn/nn_logic.h"
#include "nn/nn_player.h"

using namespace std;
namespace rn = rummy::nn;
namespace ba = boost::asio;
using Logic = shared_ptr<rn::NNLogic>;

constexpr int GENERATION_SIZE = 500;
constexpr int BAD_MOVE_REWARD = -2;

void create_init_generation(vector<Logic>& generation, const float mutationStrength, ba::thread_pool& threadPool) {
    // Resize to avoid locking later on
    generation.resize(GENERATION_SIZE);
    vector<future<void>> futures;

    for (int i = 0; i < GENERATION_SIZE; i++) {
        auto task = make_shared<packaged_task<void()>>([&generation, mutationStrength, i] {
            generation[i] = make_shared<rn::NNLogic>(mutationStrength);
        });
        futures.push_back(task->get_future());
        ba::post(threadPool, [task] {(*task)();});
    }

    for (const auto& f : futures) f.wait();
}

void load_generation_from_saves(vector<Logic>& generation, const float mutationStrength, const uint16_t num, const uint16_t introduceNew) {
    for (int i = 0; i < num; i++) {
        auto base = make_shared<rn::NNLogic>("net_" + to_string(i), mutationStrength);
        const uint16_t numChildren = (GENERATION_SIZE - introduceNew) / num - 1;

        for (int k = 0; k < numChildren; k++) {
            generation.push_back(make_shared<rummy::nn::NNLogic>(*base, mutationStrength));
        }
        generation.push_back(base);
    }

    // Add the random ones
    for (int i = generation.size(); i < GENERATION_SIZE; i++) {
        generation.push_back(make_shared<rn::NNLogic>(mutationStrength));
    }
}

// returns the score and the amount of melds played (just for progress logging)
tuple<int, int> test_networks(const Logic& a, const Logic& b) {
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

    score += (p1IsPlayer ? gs->player : gs->opponent)->print_melds().length() * 5;
    score += ((p1IsPlayer ? 1 : -1) * (gs->player->calc_points() - gs->opponent->calc_points()));
    return tuple(score, gs->melds.size());
}

void test_generation(const vector<Logic>& generation, vector<tuple<Logic, int, int>>& scoring, ba::thread_pool& threadPool) {
    vector<future<tuple<int,int>>> results;

    // Test every network against every other network twice
    for (int i = 0; i < generation.size(); i++) {
        auto task = make_shared<packaged_task<tuple<int,int>()>>([&generation, i] {
            int score = 0;
            int playedMelds = 0;
            dlib::rand rnd;
            // Clone NNLogic to avoid memory races.
            const auto a = make_shared<rn::NNLogic>(*generation[i]);
            // Play against 10% of the generation randomly
            for (int j = 0; j < GENERATION_SIZE * 0.1; j++) {

                const auto c = rnd.get_integer_in_range(0, generation.size());
                // Don't bother playing against ones self
                if (c == i) {
                    j--;
                    continue;
                }
                // Clone the NNLogic to avoid memory races.
                auto b = make_shared<rn::NNLogic>(*generation[c]);

                auto result = test_networks(a, b);
                score += get<0>(result);
                playedMelds += get<1>(result);
            }

            return tuple(score, playedMelds);
        });

        results.push_back(task->get_future());
        ba::post(threadPool, [task]{(*task)();});
    }

    for (int i = 0; i < results.size(); i++) {
        auto& fut = results[i];
        fut.wait();
        auto res = fut.get();
        scoring.push_back(tuple(generation[i], get<0>(res), get<1>(res)));
    }
}

void sim_generations(const uint16_t numGenerations, const uint16_t keepTop, const uint16_t introduceNew, const float mutationChance, const float mutationStrength, ba::thread_pool& threadPool, const bool loadLast = false) {
    vector<Logic> networks;
    cout << "Creating initial generation..." << endl;

    if (loadLast)
        load_generation_from_saves(networks, mutationStrength, keepTop, introduceNew);
    else
        create_init_generation(networks, mutationStrength, threadPool);

    cout << "done" << endl;
    for (int i = 0; i < numGenerations; i++) {
        vector<tuple<Logic, int, int>> scoring;
        cout << "Testing generation " << i << "..." << endl;
        test_generation(networks, scoring, threadPool);
        cout << "Done testing" << endl;
        partial_sort(scoring.begin(), scoring.begin() + keepTop, scoring.end(), [](const auto& a, const auto& b) {
            return get<1>(a) > get<1>(b);
        });

        // Now we evolve
        networks.clear();
        cout << "Evolving..." << endl;
        int totalScore = 0;
        int totalPlayedMelds = 0;
        for (int j = 0; j < keepTop; j++) {
            get<0>(scoring[j])->write_to_file("net_" + to_string(j));
            const uint16_t numChildren = (GENERATION_SIZE - introduceNew) / keepTop - 1;
            totalScore += get<1>(scoring[j]);
            totalPlayedMelds += get<2>(scoring[j]);
            for (int k = 0; k < numChildren; k++) {
                networks.push_back(make_shared<rummy::nn::NNLogic>(*get<0>(scoring[j]), mutationChance));
            }

            networks.push_back(get<0>(scoring[j]));
        }

        cout << "done. Average score: " << totalScore / keepTop << " played melds in top games " << totalPlayedMelds << endl;

        // Add some new completely random nets
        for (uint64_t j = networks.size(); j < GENERATION_SIZE; j++) {
            networks.push_back(make_shared<rn::NNLogic>(mutationStrength));
        }
    }
}

int main() {
    ba::thread_pool pool(std::thread::hardware_concurrency());
    sim_generations(200, 80, 25, 1, 0.01, pool);
    return 0;
}
