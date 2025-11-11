#include <iostream>

#include "game/cards.h"

int main() {
    auto p = game::getFullDeck();
    p.shuffle();

    printf("The full deck has %i cards\n", p.size());
    printf("The deck looks like: %s\n", p.toString().c_str());

    p.sort();
    printf("After sorting: %s\nThe full deck value is %i\n", p.toString().c_str(), p.getValue());

    return 0;
}