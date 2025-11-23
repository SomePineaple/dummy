//
// Created by nj60 on 11/17/25.
//

#include <iostream>
#include <tiny_dnn/tiny_dnn.h>

#include "nn/nn_logic.h"
#include "nn/nn_player.h"

using namespace std;
using namespace tiny_dnn;
using Net = tiny_dnn::network<tiny_dnn::sequential>;
using Networks = array<shared_ptr<Net>, 2>;

constexpr int GENERATION_SIZE = 100;

void create_init_generation(vector<Networks>& generation) {
    for (int i = 0; i < GENERATION_SIZE; i++) {
        Networks nets{make_shared<Net>(), make_shared<Net>()};
        (*nets[0]) << layers::fc(17, 32)
                << activation::tanh()
                << layers::fc(32, 32)
                << activation::tanh()
                << layers::fc(32, rummy::nn::CARD_EMBEDDING_SIZE);

        nets[0]->weight_init(weight_init::gaussian(2.0f));
        nets[0]->bias_init(weight_init::gaussian(2.0f));

        (*nets[1]) << layers::fc(rummy::nn::NET_INPUT_SIZE, 2400)
                  << activation::tanh()
                  << layers::fc(2400, 2400)
                  << activation::tanh()
                  << layers::fc(2400, rummy::nn::NET_OUTPUT_SIZE)
                  << activation::sigmoid();

        nets[1]->weight_init(weight_init::gaussian(2.0f));
        nets[1]->bias_init(weight_init::gaussian(2.0f));

        generation.push_back(nets);
    }
}

void test_generation(const vector<Networks>& generation, vector<tuple<Networks, int>>& scoring) {

}

int main() {
    vector<Networks> networks;
    cout << "Creating initial generation..." << endl;
    create_init_generation(networks);
    cout << "done" << endl;


    return 0;
}
