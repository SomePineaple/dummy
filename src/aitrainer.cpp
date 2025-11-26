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

// TODO: Increase generation size to 2000+, then build a rule based bot that just plays everything it gets with some randomness to test against.
// Have everyone do 5 rounds against baseline bot, filter out bottom 75%, then have the rest do 20, filter out 80% of whose remaining and run 50 rounds with top 100 who are left to choose 80 or so parents
// Same number of games as current config, but should go faster with rule bot.

using namespace std;
namespace rn = rummy::nn;
namespace ba = boost::asio;
using Logic = shared_ptr<rn::NNLogic>;

constexpr int GENERATION_SIZE = 500;
constexpr int BAD_MOVE_REWARD = -5;
constexpr uint64_t MAX_GAME_LENGTH = 100;

// Score, number of melds played, illegal move rate, score of cards not played
using metrics_t = tuple<int, uint8_t, float, uint16_t>;

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
metrics_t test_networks(const Logic& a, const Logic& b) {
    int score = 0;
    uint64_t gameLength = 0;
    uint16_t numIllegalMoves = 0;

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
            numIllegalMoves++;
        }
        swap(gs->player, gs->opponent);
        p1IsPlayer = !p1IsPlayer;
        gameLength++;
    } while (gs->player->get_hand_size() > 0 && p2->get_hand_size() > 0 && gs->stockPile.size() > 0 && gameLength < MAX_GAME_LENGTH);

    p1 = static_pointer_cast<rn::NNPlayer>(p1IsPlayer ? gs->player : gs->opponent);
    p2 = static_pointer_cast<rn::NNPlayer>(p1IsPlayer ? gs->opponent : gs->player);

    // Give a bonus for playing melds.
    score += p1->print_melds().length() * 20;
    score += p1->calc_points() - p2->calc_points();
    return metrics_t(score, p1->print_melds().size() / 3, static_cast<float>(numIllegalMoves) / gameLength, p1->get_unplayed_points());
}

void test_generation(const vector<Logic>& generation, vector<tuple<Logic, metrics_t>>& scoring, ba::thread_pool& threadPool) {
    vector<future<metrics_t>> results;

    // Test every network against every other network twice
    for (int i = 0; i < generation.size(); i++) {
        auto task = make_shared<packaged_task<metrics_t()>>([&generation, i] {
            metrics_t metrics;
            auto& [score, playedMelds, illegalRate, unplayedCards] = metrics;
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
                auto [gameScore, gamePlayedMelds, gameIllegalRate, gameUnplayedCards] = test_networks(a, b);
                score += gameScore;
                playedMelds += gamePlayedMelds;
                illegalRate += gameIllegalRate;
                unplayedCards += gameUnplayedCards;
            }

            return metrics;
        });

        results.push_back(task->get_future());
        ba::post(threadPool, [task]{(*task)();});
    }

    for (int i = 0; i < results.size(); i++) {
        auto& fut = results[i];
        fut.wait();
        auto res = fut.get();
        scoring.emplace_back(generation[i], res);
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
        vector<tuple<Logic, metrics_t>> scoring;
        cout << "Testing generation " << i << "..." << endl;
        test_generation(networks, scoring, threadPool);
        cout << "Done testing" << endl;
        partial_sort(scoring.begin(), scoring.begin() + keepTop, scoring.end(), [](const auto& a, const auto& b) {
            return get<0>(get<1>(a)) > get<0>(get<1>(b));
        });

        // Now we evolve
        networks.clear();
        cout << "Evolving..." << endl;
        int totalScore = 0;
        int totalPlayedMelds = 0;
        float totalIllegalMoveRate = 0;
        int totalUnplayedCards = 0;
        for (int j = 0; j < keepTop; j++) {
            get<0>(scoring[j])->write_to_file("net_" + to_string(j));
            const uint16_t numChildren = (GENERATION_SIZE - introduceNew) / keepTop - 1;
            auto [score, playedMelds, illegalMoveRate, unplayedCards] = get<1>(scoring[j]);
            totalScore += score;
            totalPlayedMelds += playedMelds;
            totalIllegalMoveRate += illegalMoveRate;
            totalUnplayedCards += unplayedCards;
            for (int k = 0; k < numChildren; k++) {
                networks.push_back(make_shared<rummy::nn::NNLogic>(*get<0>(scoring[j]), mutationChance));
            }

            networks.push_back(get<0>(scoring[j]));
        }

        cout << "done. Average score: " << totalScore / keepTop << ", avg num of played melds in top games " << totalPlayedMelds / (keepTop * GENERATION_SIZE * 0.1) << endl;
        cout << "Average illegal move rate: " << totalIllegalMoveRate / (keepTop * GENERATION_SIZE * 0.1) << ", average unplayed cards: " << static_cast<float>(totalUnplayedCards) / (keepTop * GENERATION_SIZE * 0.1) << endl;

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
