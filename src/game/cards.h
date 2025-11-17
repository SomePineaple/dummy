//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_CARDS_H
#define DUMMY_CARDS_H
#include <memory>
#include <vector>

namespace rummy {
    enum Suit {
        HEARTS, DIAMONDS, SPADES, CLUBS
    };

    struct Card {
        const Suit suit;
        const unsigned char value;

        [[nodiscard]] unsigned short getPointValue() const;
        [[nodiscard]] std::string toString() const;
    };

    class pile {
    public:
        pile();
        ~pile();
        void sort();
        void shuffle();
        void addCard(const std::shared_ptr<Card>& card);
        bool dump(pile& other, unsigned char numCards);
        [[nodiscard]] unsigned short calcPoints() const;
        [[nodiscard]] unsigned char size() const;
        [[nodiscard]] pile combine(const pile* pile) const;
        [[nodiscard]] std::string toString() const;
        [[nodiscard]] unsigned short getValue() const;
        [[nodiscard]] std::vector<std::shared_ptr<Card>> getCards() const;
        [[nodiscard]] std::shared_ptr<Card> getCard(unsigned char index) const;
        void removeCard(unsigned char index);
    protected:
        std::vector<std::shared_ptr<Card>> cards;
    };

    pile getFullDeck();
}

#endif //DUMMY_CARDS_H
