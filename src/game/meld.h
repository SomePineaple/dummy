//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_MELD_H
#define DUMMY_MELD_H

#include "cards.h"

namespace rummy {
    enum MeldType {
        INVALID, RUN, SET
    };

    class Meld : public Pile {
        Meld* m_BuildingFrom;
    public:
        Meld();
        [[nodiscard]] MeldType get_meld_type() const;
        bool try_build_from(Meld* other);
    };
} // game

#endif //DUMMY_MELD_H