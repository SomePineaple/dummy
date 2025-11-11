//
// Created by nj60 on 11/11/25.
//

#include "cards.h"

#include <memory>
#include <random>

namespace game {
    unsigned char getSortValue(const std::shared_ptr<Card>& card) {
        return static_cast<unsigned char>(card->suit * 13) + card->value;
    }

    bool compareCards(const std::shared_ptr<Card>& c1, const std::shared_ptr<Card>& c2) {
        return getSortValue(c1) < getSortValue(c2);
    }

    std::string Card::toString() const {
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

    Pile* getFullDeck() {
        const auto p = new Pile();

        for (unsigned char i = 1; i <= 13; i++) {
            p->addCard(std::make_shared<Card>(Card{SPADES, i}));
            p->addCard(std::make_shared<Card>(Card{HEARTS, i}));
            p->addCard(std::make_shared<Card>(Card{CLUBS, i}));
            p->addCard(std::make_shared<Card>(Card{DIAMONDS, i}));
        }

        return p;
    }

    Pile::Pile() {
        cards = std::vector<std::shared_ptr<Card>>{};
    }

    Pile::~Pile() = default;

    void Pile::addCard(const std::shared_ptr<Card>& card) {
        cards.push_back(card);
    }

    void Pile::sort() {
        std::sort(cards.begin(), cards.end(), compareCards);
    }

    void Pile::shuffle() {
        std::random_device rd;
        std::mt19937 g(rd());

        std::shuffle(cards.begin(),cards.end(), g);
    }

    unsigned char Pile::size() const {
        return cards.size();
    }

    std::string Pile::toString() const {
        std::string str;
        for (const auto& c : cards)
            str += c->toString() + " ";

        return str;
    }
}
