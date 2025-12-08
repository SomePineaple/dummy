//
// Created by nj60 on 11/11/25.
//

#include "cards.h"

#include <random>
#include <algorithm>
#include "../nn/nn_logic.h"

namespace rummy {
    bool cmp_cards(const Card& c1, const Card& c2) {
        return c1.get_sort_value() < c2.get_sort_value();
    }

    uint8_t Card::get_sort_value() const {
        return static_cast<uint8_t>(suit) * 13 + value;
    }


    std::string Card::to_string() const {
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
            case Suit::CLUBS:
                str += "C";
                break;
            case Suit::DIAMONDS:
                str += "D";
                break;
            case Suit::SPADES:
                str += "S";
                break;
            case Suit::HEARTS:
                str += "H";
                break;
        }

        return str;
    }

    uint16_t Card::get_point_value() const {
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

    nn::card_input_t Card::one_hot() const {
        nn::card_input_t v{};

        v = 0;

        v(0, static_cast<int64_t>(suit)) = 1;
        v(0, 3 + value) = 1;

        return v;
    }

    bool Card::operator==(const Card &other) const {
        return value == other.value && suit == other.suit;
    }

    Pile get_full_deck() {
        Pile p{};

        for (unsigned char i = 1; i <= 13; i++) {
            p.add_card({Suit::SPADES, i});
            p.add_card({Suit::HEARTS, i});
            p.add_card({Suit::CLUBS, i});
            p.add_card({Suit::DIAMONDS, i});
        }

        return p;
    }

    void Pile::add_card(const Card& card) {
        m_cards.push_back(card);
    }

    bool Pile::dump(Pile &other, const unsigned char numCards) {
        if (numCards > m_cards.size())
            return false;

        for (int i = 0; i < numCards; i++) {
            other.m_cards.push_back(m_cards.back());
            m_cards.pop_back();
        }

        return true;
    }

    Pile Pile::combine(const Pile* pile) const {
        auto p = Pile{};
        p.m_cards.insert(p.m_cards.begin(), m_cards.begin(), m_cards.end());
        p.m_cards.insert(p.m_cards.end(), pile->m_cards.begin(), pile->m_cards.end());

        return p;
    }


    void Pile::sort() {
        std::sort(m_cards.begin(), m_cards.end(), cmp_cards);
    }

    void Pile::shuffle() {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(m_cards.begin(),m_cards.end(), g);
    }


    uint8_t Pile::size() const {
        return m_cards.size();
    }

    std::string Pile::to_string() const {
        return std::accumulate(m_cards.begin(), m_cards.end(), std::string{}, [](const auto& str, const auto& card) {
            return str + " " + card.to_string();
        });
    }

    uint16_t Pile::get_value() const {
        return std::accumulate(m_cards.begin(), m_cards.end(), 0u, [](uint16_t sum, const auto& card) {
            return sum + card.get_point_value();
        });
    }

    std::vector<Card> Pile::get_cards() const {
        std::vector c(m_cards);
        return c;
    }

    Card Pile::get_card(const unsigned char index) const {
        if (index > m_cards.size())
            throw std::runtime_error("Tried to get a card with an out of bounds index");
        return m_cards.at(index);
    }

    void Pile::set_cards(const std::vector<Card>& cards) {
        m_cards.clear();
        m_cards.insert(m_cards.begin(), cards.begin(), cards.end());
    }


    void Pile::remove_at(const unsigned char index) {
        // TODO: Look into if a std::list would be a better option given the fact that this operation is very expensive
        m_cards.erase(m_cards.begin() + index);
    }
}
