//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_MELD_H
#define DUMMY_MELD_H

#include "cards.h"

namespace rummy {
    enum meld_type {
        INVALID, RUN, SET
    };

    class meld : public pile {
        meld* buildingFrom;
    public:
        meld();
        [[nodiscard]] meld_type meld_type() const;
        bool try_build_from(meld* building_from);
    };
} // game

#endif //DUMMY_MELD_H