//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_CARDS_H
#define DUMMY_CARDS_H
#include <memory>
#include <vector>

namespace game {
    enum Suit {
        HEARTS, DIAMONDS, SPADES, CLUBS
    };

    struct Card {
        const Suit suit;
        const unsigned char value;

        [[nodiscard]] std::string toString() const;
    };

    class Pile {
    public:
        Pile();
        ~Pile();
        void sort();
        void shuffle();
        void addCard(const std::shared_ptr<Card>& card);
        void dump();
        [[nodiscard]] unsigned char size() const;
        [[nodiscard]] Pile combine(const Pile& pile) const;
        [[nodiscard]] std::string toString() const;
    private:
        std::vector<std::shared_ptr<Card>> cards;
    };

    Pile* getFullDeck();
}

#endif //DUMMY_CARDS_H