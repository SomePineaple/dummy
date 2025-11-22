//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_CARDS_H
#define DUMMY_CARDS_H
#include <memory>
#include <vector>
#include <cstdint>

namespace rummy {
    using namespace std;
    enum Suit {
        HEARTS, DIAMONDS, SPADES, CLUBS
    };

    struct Card {
        const Suit suit;
        const uint8_t value;

        [[nodiscard]] uint16_t get_point_value() const;
        [[nodiscard]] uint8_t get_sort_value() const;
        [[nodiscard]] string to_string() const;
        [[nodiscard]] std::array<float, 17> one_hot() const;
    };

    class Pile {
    public:
        Pile();
        ~Pile();

        void sort();
        void shuffle();
        void add_card(const shared_ptr<Card>& card);
        bool dump(Pile& other, uint8_t numCards);

        [[nodiscard]] uint16_t calc_points() const;
        [[nodiscard]] uint8_t size() const;
        [[nodiscard]] Pile combine(const Pile* pile) const;
        [[nodiscard]] string to_string() const;
        [[nodiscard]] uint16_t get_value() const;
        [[nodiscard]] vector<shared_ptr<Card>> get_cards() const;
        [[nodiscard]] shared_ptr<Card> get_card(uint8_t index) const;
        void remove_at(uint8_t index);
    protected:
        vector<shared_ptr<Card>> m_cards;
    };

    Pile get_full_deck();
}

#endif //DUMMY_CARDS_H
