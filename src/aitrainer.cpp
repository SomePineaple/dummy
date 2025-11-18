//
// Created by nj60 on 11/17/25.
//

#include <iostream>
#include <tiny_dnn/tiny_dnn.h>

#include "game/cards.h"
#include "nn/nn_logic.h"

int main() {
    rummy::card c{rummy::DIAMONDS, 11};

    tiny_dnn::network<tiny_dnn::sequential> net;

    net << tiny_dnn::layers::fc(17, 32)
        << tiny_dnn::activation::tanh()
        << tiny_dnn::layers::fc(32, 32)
        << tiny_dnn::activation::tanh()
        << tiny_dnn::layers::fc(32, rummy::nn::CARD_EMBEDDING_SIZE);

    net.init_weight();
    net.bias_init(tiny_dnn::weight_init::xavier());

    auto data = c.one_hot();

    auto out = net.predict(data);
    for (int i = 0; i < rummy::nn::CARD_EMBEDDING_SIZE; i++) {
        std::cout << out[i] << " ";
    }

    std::cout << std::endl;

    return 0;
}
