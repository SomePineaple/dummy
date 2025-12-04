//
// Created by nj60 on 12/3/25.
//

#include "cpu_trainer.h"

#include <boost/asio/post.hpp>
#include "../nn_player.h"
#include "../../game/clients/rule_bot.h"

namespace ba = boost::asio;

namespace rummy::nn {
    CpuTrainer::CpuTrainer(const uint16_t maxGameLength, const uint16_t generationSize, const uint16_t badMoveReward, const uint32_t maxThreads, const float mutationChance, const float mutationStrength) :
        m_MaxGameLength(maxGameLength), m_GenerationSize(generationSize), m_BadMoveReward(badMoveReward), m_MutationChance(mutationChance), m_MutationStrength(mutationStrength), m_ThreadPool(maxThreads)
    {
        m_networks.resize(generationSize);
        std::vector<std::future<void>> futures;

        for (uint16_t i = 0; i < generationSize; i++) {
            auto task = make_shared<packaged_task<void()>>([this, mutationStrength, i] {
                m_networks[i] = std::make_shared<NNLogic>(mutationStrength);
            });
            futures.push_back(task->get_future());
            ba::post(m_ThreadPool, [task] {(*task)();});
        }

        for (const auto& f : futures) f.wait();
    }

    CpuTrainer::~CpuTrainer() {
        m_ThreadPool.join();
    }

    // returns the score and the amount of melds played (just for progress logging)
    metrics_t CpuTrainer::test_networks(const Logic& net) const {
        int score = 0;
        uint64_t gameLength = 0;
        uint16_t numIllegalMoves = 0;

        auto p1 = make_shared<NNPlayer>(net);
        auto p2 = make_shared<clients::RuleBot>();

        auto gs = std::make_unique<GameState>(p1, p2);

        bool p1IsPlayer = true;

        do {
            auto backupGs = std::make_unique<GameState>(gs.get());
            if (!gs->player->run_turn(gs.get())) {
                swap(gs, backupGs);
                static_pointer_cast<NNPlayer>(gs->player)->random_turn(gs.get());
                score += m_BadMoveReward;
                numIllegalMoves++;
            }

            swap(gs->player, gs->opponent);
            p1IsPlayer = !p1IsPlayer;
            gameLength++;

        } while (gs->player->get_hand_size() > 0 && p2->get_hand_size() > 0 && gs->stockPile.size() > 0 && gameLength < m_MaxGameLength);

        p1 = static_pointer_cast<NNPlayer>(p1IsPlayer ? gs->player : gs->opponent);
        //p2 = static_pointer_cast<rn::NNPlayer>(p1IsPlayer ? gs->opponent : gs->player);

        // Give a bonus for playing melds.
        score += static_cast<int>(p1->print_melds().length()) * 20;
        score += p1->calc_points() - p2->calc_points();
        return {score, p1->print_melds().size() / 3.0, static_cast<float>(numIllegalMoves) / gameLength, p1->get_unplayed_points()};
    }

    void CpuTrainer::test_generation() {
        m_scoring.clear();
        std::vector<std::future<metrics_t>> results;

        // Test every network against every other network twice
        for (const auto& network : m_networks) {
            auto task = std::make_shared<packaged_task<metrics_t()>>([this, &network] {
                metrics_t metrics;
                auto& [score, playedMelds, illegalRate, unplayedCards] = metrics;
                dlib::rand rnd;
                // Clone NNLogic to avoid memory races.
                const auto a = std::make_shared<NNLogic>(*network);
                // Play against rule bot 25 times.
                for (int j = 0; j < 25; j++) {
                    auto [gameScore, gamePlayedMelds, gameIllegalRate, gameUnplayedCards] = test_networks(a);
                    score += gameScore;
                    playedMelds += gamePlayedMelds;
                    illegalRate += gameIllegalRate;
                    unplayedCards += gameUnplayedCards;
                }

                return metrics;
            });

            results.push_back(task->get_future());
            ba::post(m_ThreadPool, [task]{(*task)();});
        }

        for (int i = 0; i < results.size(); i++) {
            auto& fut = results[i];
            fut.wait();
            auto res = fut.get();
            m_scoring.emplace_back(m_networks[i], res);
        }
    }

    void CpuTrainer::evolve(const uint16_t keepTop, const uint16_t introduceNew) {
        partial_sort(m_scoring.begin(), m_scoring.begin() + keepTop, m_scoring.end(), [](const auto& a, const auto& b) {
            return get<0>(get<1>(a)) > get<0>(get<1>(b));
        });

        // Now we evolve
        m_networks.clear();
        cout << "Evolving..." << endl;
        int totalScore = 0;
        float totalPlayedMelds = 0;
        float totalIllegalMoveRate = 0;
        int totalUnplayedCards = 0;

        std::vector<std::future<std::vector<shared_ptr<NNLogic>>>> futures;

        for (int j = 0; j < keepTop; j++) {
            const uint16_t numChildren = (m_GenerationSize - introduceNew) / keepTop - 1;
            auto [score, playedMelds, illegalMoveRate, unplayedCards] = get<1>(m_scoring[j]);
            totalScore += score;
            totalPlayedMelds += playedMelds;
            totalIllegalMoveRate += illegalMoveRate;
            totalUnplayedCards += unplayedCards;

            auto task = std::make_shared<packaged_task<std::vector<shared_ptr<NNLogic>>()>>([&] {
                std::vector<shared_ptr<NNLogic>> nets;
                for (int k = 0; k < numChildren; k++) {
                    nets.push_back(std::make_shared<NNLogic>(*get<0>(m_scoring[j]), m_MutationChance));
                }

                nets.push_back(get<0>(m_scoring[j]));
                return nets;
            });

            futures.push_back(task->get_future());
            ba::post(m_ThreadPool, [task]{ (*task)(); });
        }

        cout << "Average score: " << totalScore / keepTop << ", avg num of played cards in top games " << totalPlayedMelds / (keepTop * 25) << endl;
        cout << "Average illegal move rate: " << totalIllegalMoveRate / (keepTop * 25) << ", average unplayed points: " << static_cast<float>(totalUnplayedCards) / (keepTop * 25) << endl;

        for (auto& fut : futures) {
            fut.wait();
            auto res = fut.get();
            m_networks.insert(m_networks.end(), res.begin(), res.end());
        }

        // Add some new completely random nets
        for (uint64_t j = m_networks.size(); j < m_GenerationSize; j++) {
            m_networks.push_back(make_shared<NNLogic>(m_MutationStrength));
        }
    }

    void CpuTrainer::save_top(const uint16_t num) {
        const uint16_t saveTop = std::min(num, m_GenerationSize);

        partial_sort(m_scoring.begin(), m_scoring.begin() + saveTop, m_scoring.end(), [](const auto& a, const auto& b) {
            return get<0>(get<1>(a)) > get<0>(get<1>(b));
        });

        for (uint16_t i = 0; i < saveTop; i++) {
            get<0>(m_scoring[i])->write_to_file("net_" + to_string(i));
        }
    }
}
