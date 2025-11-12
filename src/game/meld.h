//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_MELD_H
#define DUMMY_MELD_H

#include <optional>

#include "cards.h"

namespace game {
    enum MeldType {
        INVALID, RUN, SET
    };

    class Meld : Pile {
        std::optional<Meld*> buildingFrom;
    public:
        Meld();
        Meld(Meld& buildingFrom);
        [[nodiscard]] MeldType getMeldType() const;
    };
} // game

#endif //DUMMY_MELD_H