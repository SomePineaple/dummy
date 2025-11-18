//
// Created by nj60 on 11/17/25.
//

#include <iostream>
#include <tiny_dnn/tiny_dnn.h>

#include "game/game.h"
#include "nn/nn_logic.h"
#include "nn/nn_player.h"

int main() {
    tiny_dnn::network<tiny_dnn::sequential> embedder;

    std::cout << "Building networks..." << std::endl;
    embedder << tiny_dnn::layers::fc(17, 32)
        << tiny_dnn::activation::tanh()
        << tiny_dnn::layers::fc(32, 32)
        << tiny_dnn::activation::tanh()
        << tiny_dnn::layers::fc(32, rummy::nn::CARD_EMBEDDING_SIZE);

    embedder.init_weight();
    embedder.bias_init(tiny_dnn::weight_init::xavier());

    tiny_dnn::network<tiny_dnn::sequential> actor;

    actor << tiny_dnn::layers::fc(rummy::nn::NET_INPUT_SIZE, 2400)
          << tiny_dnn::activation::tanh()
          << tiny_dnn::layers::fc(2400, 2400)
          << tiny_dnn::activation::tanh()
          << tiny_dnn::layers::fc(2400, rummy::nn::NET_OUTPUT_SIZE)
          << tiny_dnn::activation::sigmoid();
    actor.init_weight();
    actor.bias_init(tiny_dnn::weight_init::xavier());

    std::cout << "done." << std::endl;

    const rummy::game_state gs{make_shared<rummy::nn::nn_player>(embedder, actor), make_shared<rummy::nn::nn_player>(embedder, actor)};

    std::cout << "creating logic and initializing game state..." << std::endl;
    rummy::nn::nn_logic logic{embedder, actor};
    logic.init_gs(&gs);
    cout << "done" << std::endl;

    cout << "we should draw number: " << static_cast<int>(logic.get_draw()) << " and discard number " << static_cast<int>(logic.get_discard()) << endl;

    return 0;
}
