//
// Created by nj60 on 12/4/25.
//

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

#include <nn/nn_logic.h>

#include <fstream>

#include "nn/training/cpu_trainer.h"

bool files_are_equal(const std::string& path1, const std::string& path2) {
    std::ifstream f1(path1, std::ios::binary);
    std::ifstream f2(path2, std::ios::binary);
    if (!f1 || !f2) return false;               // one of the files couldn't be opened

    // Optional fast‑path: compare sizes first
    f1.seekg(0, std::ios::end);
    f2.seekg(0, std::ios::end);
    if (f1.tellg() != f2.tellg()) return false;
    f1.seekg(0);
    f2.seekg(0);

    constexpr std::size_t bufferSize = 8192;
    char buf1[bufferSize];
    char buf2[bufferSize];

    while (f1 && f2) {
        f1.read(buf1, bufferSize);
        f2.read(buf2, bufferSize);
        std::streamsize n1 = f1.gcount();
        std::streamsize n2 = f2.gcount();

        if (n1 != n2) return false;            // should never happen after size check
        if (!std::equal(buf1, buf1 + n1, buf2)) return false;
    }
    return true;
}

BOOST_AUTO_TEST_CASE(testEvolution) {
    rummy::nn::NNLogic logic(0.1);
    logic.write_to_file("1");

    rummy::nn::NNLogic logic2(logic, 1);
    logic2.write_to_file("2");

    // If we really evolved, we should get some different output this time.
    BOOST_CHECK(!files_are_equal("1_actor.bin", "2_actor.bin"));
    BOOST_CHECK(!files_are_equal("1_embedder.bin", "2_embedder.bin"));
}

#ifdef PROFILE
#include <ctrack.hpp>
BOOST_AUTO_TEST_CASE(simpleProfiling) {
    std::cout << "Running 5 generations to profile..." << std::endl;

    rummy::nn::CpuTrainer trainer(50, 50, -5, std::thread::hardware_concurrency(), 1, 0.05);
    for (int i = 0; i < 5; i++) {
        trainer.test_generation();
        trainer.evolve(10, 10);
    }

    ctrack::result_print();
}

#endif
