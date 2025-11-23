//
// Created by nj60 on 11/17/25.
//

#include <iostream>
#include <memory>
#include <tiny_dnn/tiny_dnn.h>
#include <thread>
#include <mutex>

#include "nn/nn_logic.h"
#include "nn/nn_player.h"

using namespace std;
using namespace tiny_dnn;
using Net = tiny_dnn::network<tiny_dnn::sequential>;
using Logic = shared_ptr<rummy::nn::NNLogic>;

constexpr int GENERATION_SIZE = 20;
constexpr int BAD_MOVE_REWARD = -2;

void create_init_generation(vector<Logic>& generation) {
    vector<thread> threads;
    mutex generationLock;

    for (int i = 0; i < GENERATION_SIZE; i++) {
        threads.emplace_back([&generation, &generationLock] {
            auto embedder = make_shared<Net>();
            (*embedder) << layers::fc(17, 32)
                    << activation::tanh()
                    << layers::fc(32, 32)
                    << activation::tanh()
                    << layers::fc(32, rummy::nn::CARD_EMBEDDING_SIZE);

            embedder->weight_init(weight_init::gaussian(2.0f));
            embedder->bias_init(weight_init::gaussian(2.0f));

            auto actor = make_shared<Net>();
            (*actor) << layers::fc(rummy::nn::NET_INPUT_SIZE, 2400)
                      << activation::tanh()
                      << layers::fc(2400, 2400)
                      << activation::tanh()
                      << layers::fc(2400, rummy::nn::NET_OUTPUT_SIZE)
                      << activation::sigmoid();

            actor->weight_init(weight_init::gaussian(2.0f));
            actor->bias_init(weight_init::gaussian(2.0f));

            lock_guard lock(generationLock);
            generation.push_back(make_shared<rummy::nn::NNLogic>(embedder, actor));
        });
    }

    for_each(threads.begin(), threads.end(), [](auto& thread) {thread.join();});
}

int test_networks(const Logic& a, const Logic& b) {
    int score = 0;

    auto p1 = make_shared<rummy::nn::NNPlayer>(a);
    auto p2 = make_shared<rummy::nn::NNPlayer>(b);

    auto gs = std::make_unique<rummy::GameState>(p1, p2);

    bool p1IsPlayer = true;

    do {
        auto backupGs = std::make_unique<rummy::GameState>(gs.get());
        if (!gs->player->run_turn(gs.get())) {
            swap(gs, backupGs);
            static_pointer_cast<rummy::nn::NNPlayer>(gs->player)->random_turn(gs.get());
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
            cout << "Testing number: " << i << " ..." << endl;
            for (int f = i; f < i +generation.size(); f++) {
                const uint64_t j = f % generation.size();
                if (i == j) continue;

                Logic a{generation[i]};
                Logic b{generation[j]};

                score += test_networks(a, b);
            }
            cout << "Number " << i << " scored " << score << endl;

            lock_guard scoringLock(scoringMutex);
            scoring.emplace_back(generation[i], score);
        });
    }

    for_each(threads.begin(), threads.end(), [](auto& thread) {thread.join();});
}

int main() {
    vector<Logic> networks;
    cout << "Creating initial generation..." << endl;
    create_init_generation(networks);
    cout << "done" << endl;
    vector<tuple<Logic, int>> scoring;
    cout << "Testing..." << endl;
    test_generation(networks, scoring);
    cout << "Done testing" << endl;

    return 0;
}
