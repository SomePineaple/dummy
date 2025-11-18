//
// Created by nj60 on 11/11/25.
//

#ifndef DUMMY_CARDS_H
#define DUMMY_CARDS_H
#include <memory>
#include <vector>

namespace rummy {
    enum suit_type {
        HEARTS, DIAMONDS, SPADES, CLUBS
    };

    struct card {
        const suit_type suit;
        const unsigned char value;

        card(const suit_type suit, const unsigned char value) : suit(suit), value(value) {}
        [[nodiscard]] unsigned short get_point_value() const;
        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] std::array<float, 17> one_hot() const;
    };

    class pile {
    public:
        pile();
        ~pile();
        void sort();
        void shuffle();
        void add_card(const std::shared_ptr<card>& card);
        bool dump(pile& other, unsigned char numCards);
        [[nodiscard]] unsigned short calc_points() const;
        [[nodiscard]] unsigned char size() const;
        [[nodiscard]] pile combine(const pile* other) const;
        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] std::vector<std::shared_ptr<card>> get_cards() const;
        [[nodiscard]] std::shared_ptr<card> get_card(unsigned char index) const;
        void remove_card(unsigned char index);
    protected:
        std::vector<std::shared_ptr<card>> cards;
    };

    pile get_full_deck();
} // rummy

#endif //DUMMY_CARDS_H
