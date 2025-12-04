//
// Created by nj60 on 12/3/25.
//

#ifndef DUMMY_CPU_TRAINER_H
#define DUMMY_CPU_TRAINER_H

#include <vector>
#include <memory>
#include <boost/asio/thread_pool.hpp>

#include "../nn_logic.h"

namespace rummy::nn {
    // Score, number of melds played, illegal move rate, score of cards not played
    using metrics_t = std::tuple<int16_t, float, float, uint16_t>;
    using Logic = std::shared_ptr<NNLogic>;

    class CpuTrainer {
        metrics_t test_networks(const Logic& net) const;

        uint16_t m_MaxGameLength;
        uint16_t m_GenerationSize;
        uint16_t m_BadMoveReward;
        float m_MutationChance;
        float m_MutationStrength;

        std::vector<Logic> m_networks;
        std::vector<std::tuple<Logic, metrics_t>> m_scoring;
        boost::asio::thread_pool m_ThreadPool;
    public:
        CpuTrainer(uint16_t maxGameLength, uint16_t generationSize, uint16_t badMoveReward, uint32_t maxThreads, float mutationChance, float mutationStrength);
        ~CpuTrainer();

        void test_generation();
        void evolve(uint16_t keepTop, uint16_t introduceNew);
    };
}


#endif //DUMMY_CPU_TRAINER_H
