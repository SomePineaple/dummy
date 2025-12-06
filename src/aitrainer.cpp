//
// Created by nj60 on 11/17/25.
//

#include <thread>
#include "nn/training/cpu_trainer.h"

// TODO: Increase generation size to 2000+, then build a rule based bot that just plays everything it gets with some randomness to test against.
// Have everyone do 5 rounds against baseline bot, filter out bottom 75%, then have the rest do 20, filter out 80% of whose remaining and run 50 rounds with top 100 who are left to choose 80 or so parents
// Same number of games as current config, but should go faster with rule bot.

// TODO: Add unit testing to make sure that evolving and mutating is actually working.

constexpr uint16_t GENERATION_SIZE = 1000;
constexpr int BAD_MOVE_REWARD = -50;
constexpr uint64_t MAX_GAME_LENGTH = 100;
constexpr uint16_t NUM_GENERATIONS = 200;

/* load_generation_from_saves(vector<Logic>& generation, const float mutationStrength, const uint16_t num, const uint16_t introduceNew) {
    for (int i = 0; i < num; i++) {
        auto base = make_shared<rn::NNLogic>("net_" + to_string(i), mutationStrength);
        const uint16_t numChildren = (GENERATION_SIZE - introduceNew) / num - 1;

        for (int k = 0; k < numChildren; k++) {
            generation.push_back(make_shared<rummy::nn::NNLogic>(*base, mutationStrength));
        }
        generation.push_back(base);
    }

    // Add the random ones
    for (size_t i = generation.size(); i < GENERATION_SIZE; i++) {
        generation.push_back(make_shared<rn::NNLogic>(mutationStrength));
    }
}*/

int main() {
    rummy::nn::CpuTrainer trainer(MAX_GAME_LENGTH, GENERATION_SIZE, BAD_MOVE_REWARD, std::thread::hardware_concurrency(), 1, 0.05);
    for (uint16_t i = 0; i < NUM_GENERATIONS; i++) {
        std::cout << "Testing generation " << i << std::endl;
        trainer.test_generation();
        trainer.evolve(GENERATION_SIZE * 0.1, GENERATION_SIZE * 0.1);

        // Save the top 5 networks every 10 generations
        if (!(i % 10)) {
            std::cout << "Saving the top 5 networks to a file..." << std::endl;
            trainer.save_top(5);
            std::cout << "done." << std::endl;
        }
    }
    return 0;
}
