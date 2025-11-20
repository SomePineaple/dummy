//
// Created by nj60 on 11/11/25.
//

#include "cards.h"

#include <memory>
#include <random>
#include <algorithm>

namespace rummy {
    unsigned char getSortValue(const std::shared_ptr<Card>& card) {
        return static_cast<unsigned char>(card->suit * 13) + card->value;
    }

    bool compareCards(const std::shared_ptr<Card>& c1, const std::shared_ptr<Card>& c2) {
        return getSortValue(c1) < getSortValue(c2);
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

    unsigned short Card::get_point_value() const {
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


    Pile get_full_deck() {
        Pile p{};

        for (unsigned char i = 1; i <= 13; i++) {
            p.add_card(std::make_shared<Card>(Card{SPADES, i}));
            p.add_card(std::make_shared<Card>(Card{HEARTS, i}));
            p.add_card(std::make_shared<Card>(Card{CLUBS, i}));
            p.add_card(std::make_shared<Card>(Card{DIAMONDS, i}));
        }

        return p;
    }

    Pile::Pile() {
        m_cards = std::vector<std::shared_ptr<Card>>{};
    }

    Pile::~Pile() = default;

    void Pile::add_card(const std::shared_ptr<Card>& card) {
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
        std::sort(m_cards.begin(), m_cards.end(), compareCards);
    }

    void Pile::shuffle() {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(m_cards.begin(),m_cards.end(), g);
    }

    unsigned short Pile::calc_points() const {
        return std::accumulate(m_cards.begin(), m_cards.end(), 0u, [](const auto& sum, const auto& card) {
            return sum + card->get_point_value();
        });
    }

    unsigned char Pile::size() const {
        return m_cards.size();
    }

    std::string Pile::to_string() const {
        return std::accumulate(m_cards.begin(), m_cards.end(), std::string{}, [](const auto& str, const auto& card) {
            return str + " " + card->to_string();
        });
    }

    unsigned short Pile::get_value() const {
        return std::accumulate(m_cards.begin(), m_cards.end(), 0u, [](unsigned short sum, const auto& card) {
            return sum + card->get_point_value();
        });
    }

    std::vector<std::shared_ptr<Card>> Pile::get_cards() const {
        std::vector c(m_cards);
        return c;
    }

    std::shared_ptr<Card> Pile::get_card(const unsigned char index) const {
        return m_cards.at(index);
    }

    void Pile::remove_at(const unsigned char index) {
        // TODO: Look into if a std::list would be a better option given the fact that this operation is very expensive
        m_cards.erase(m_cards.begin() + index);
    }
}
