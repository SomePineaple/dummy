#include <iostream>

#include "game/cards.h"

int main() {
    const auto p = game::getFullDeck();
    p->shuffle();

    printf("The full deck has %i cards\n", p->size());
    printf("The deck looks like: %s\n", p->toString().c_str());

    p->sort();
    printf("After sorting: %s\n", p->toString().c_str());

    delete p;

    return 0;
}