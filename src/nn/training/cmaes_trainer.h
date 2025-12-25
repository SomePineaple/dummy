//
// Created by nj60 on 12/24/25.
//

#ifndef DUMMY_CMAES_TRAINER_H
#define DUMMY_CMAES_TRAINER_H

#include <cstdint>

namespace rummy::nn {
    class CmaesTrainer {
    public:
        CmaesTrainer();

        void train();
        void saveTop(uint8_t numSaved);
    private:

    };
}

#endif //DUMMY_CMAES_TRAINER_H
