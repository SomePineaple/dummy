//
// Created by nj60 on 11/11/25.
//

#include "cards.h"

#include <memory>
#include <random>
#include <algorithm>

namespace rummy {
    unsigned char get_sort_value(const std::shared_ptr<card>& card) {
        return static_cast<unsigned char>(card->suit * 13) + card->value;
    }

    bool compare_cards(const std::shared_ptr<card>& c1, const std::shared_ptr<card>& c2) {
        return get_sort_value(c1) < get_sort_value(c2);
    }

    std::string card::to_string() const {
        std::string str;

        switch (value) {
            case 1:
                str = "A";
                break;
            case 11:
                str = "J";
                break;
            case 12:
                str = "Q";
                break;
            case 13:
                str = "K";
                break;
            default:
                str = std::to_string(value);
                break;
        }

        switch (suit) {
            case CLUBS:
                str += "C";
                break;
            case DIAMONDS:
                str += "D";
                break;
            case SPADES:
                str += "S";
                break;
            case HEARTS:
                str += "H";
                break;
        }

        return str;
    }

    unsigned short card::get_point_value() const {
        switch (value) {
            case 1:
                return 15;
            case 11:
            case 12:
            case 13:
                return 10;
            default:
                return value;
        }
    }

    std::array<float, 17> card::one_hot() const {
        std::array<float, 17> v{};
        v.fill(0);
        v[suit] = 1;
        v[3 + value] = 1;

        return v;
    }



    pile get_full_deck() {
        pile p{};

        for (unsigned char i = 1; i <= 13; i++) {
            p.add_card(std::make_shared<card>(SPADES, i));
            p.add_card(std::make_shared<card>(HEARTS, i));
            p.add_card(std::make_shared<card>(CLUBS, i));
            p.add_card(std::make_shared<card>(DIAMONDS, i));
        }

        return p;
    }

    pile::pile() {
        cards = std::vector<std::shared_ptr<card>>{};
    }

    pile::~pile() = default;

    void pile::add_card(const std::shared_ptr<card>& card) {
        cards.push_back(card);
    }

    bool pile::dump(pile &other, const unsigned char numCards) {
        if (numCards > cards.size())
            return false;

        for (int i = 0; i < numCards; i++) {
            other.cards.push_back(cards.back());
            cards.pop_back();
        }

        return true;
    }

    pile pile::combine(const pile* other) const {
        auto p = pile{};
        p.cards.insert(p.cards.begin(), cards.begin(), cards.end());
        p.cards.insert(p.cards.end(), other->cards.begin(), other->cards.end());

        return p;
    }


    void pile::sort() {
        std::sort(cards.begin(), cards.end(), compare_cards);
    }

    void pile::shuffle() {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(cards.begin(),cards.end(), g);
    }

    unsigned short pile::calc_points() const {
        return std::accumulate(cards.begin(), cards.end(), 0u, [](const auto& sum, const auto& card) {
            return sum + card->get_point_value();
        });
    }

    unsigned char pile::size() const {
        return cards.size();
    }

    std::string pile::to_string() const {
        return std::accumulate(cards.begin(), cards.end(), std::string{}, [](const auto& str, const auto& card) {
            return str + " " + card->to_string();
        });
    }

    std::vector<std::shared_ptr<card>> pile::get_cards() const {
        std::vector c(cards);
        return c;
    }

    std::shared_ptr<card> pile::get_card(const unsigned char index) const {
        return cards.at(index);
    }

    void pile::remove_card(const unsigned char index) {
        // TODO: Look into if a std::list would be a better option given the fact that this operation is very expensive
        cards.erase(cards.begin() + index);
    }
}
