//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_CARDS_H
#define DUMMY_CARDS_H

#include <vector>

#include <dlib/matrix.h>

namespace rummy {
    enum class Suit {
        HEARTS, DIAMONDS, SPADES, CLUBS
    };

    struct Card {
        Suit suit;
        uint8_t value;

        [[nodiscard]] uint16_t get_point_value() const;
        [[nodiscard]] uint8_t get_sort_value() const;
        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] dlib::matrix<float, 1, 17> one_hot() const;

        bool operator==(const Card& other) const;
    };

    class Pile {
    public:
        void sort();
        void shuffle();
        void add_card(const Card& card);
        bool dump(Pile& other, uint8_t numCards);

        [[nodiscard]] uint8_t size() const;
        [[nodiscard]] Pile combine(const Pile* pile) const;
        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] uint16_t get_value() const;
        [[nodiscard]] virtual std::vector<Card> get_cards() const;
        [[nodiscard]] Card get_card(uint8_t index) const;

        void set_cards(const std::vector<Card>& cards);

        void remove_at(uint8_t index);
    protected:
        std::vector<Card> m_cards;
    };

    Pile get_full_deck();
}

#endif //DUMMY_CARDS_H
