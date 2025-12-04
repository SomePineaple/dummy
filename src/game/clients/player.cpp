//
// Created by nj60 on 11/12/25.
//

#include "player.h"

#include <numeric>
#include "../game.h"

namespace rummy::clients {
    bool Player::draw_from_stock(GameState* gs, const uint8_t numCards) {
        return gs->stockPile.dump(m_hand, numCards);
    }

    bool Player::draw_from_discard(GameState* gs, const uint8_t numCards) {
        if (gs->discardPile.dump(m_hand, numCards)) {
            // If we draw more than one from discard, we have to play it
            if (numCards != 1)
                m_hand.dump(m_WorkingMeld, 1);
            return true;
        }
        return false;
    }

    bool Player::play_working_meld(GameState* gs) {
        if (m_WorkingMeld.size() == 0)
            return false;

        if (m_WorkingMeld.size() < 3) {
            for (const auto& m : gs->melds) {
                if (m_WorkingMeld.try_build_from(m.get())) {
                    m_PlayedMelds.push_back(std::make_shared<Meld>(m_WorkingMeld));
                    gs->melds.push_back(m_PlayedMelds.back());
                    m_WorkingMeld = Meld{};
                    return true;
                }
            }
        } else if (m_WorkingMeld.get_meld_type() != INVALID) {
            m_PlayedMelds.push_back(std::make_shared<Meld>(m_WorkingMeld));
            gs->melds.push_back(m_PlayedMelds.back());
            m_WorkingMeld = Meld{};
            return true;
        }

        return false;
    }

    bool Player::discard(GameState* gs, const uint8_t cardNumber) {
        if (cardNumber >= m_hand.size())
            return false;
        gs->discardPile.add_card(m_hand.get_card(cardNumber));
        m_hand.remove_at(cardNumber);

        return true;
    }

    bool Player::add_to_working_meld(const uint8_t cardNumber) {
        if (cardNumber >= m_hand.size())
            return false;

        m_WorkingMeld.add_card(m_hand.get_card(cardNumber));
        m_hand.remove_at(cardNumber);

        return true;
    }

    int16_t Player::calc_points() {
        int16_t sum = std::accumulate(m_PlayedMelds.begin(),m_PlayedMelds.end(), 0, [](const auto& s, const auto& m) {
            return s + m->get_value();
        });

        sum -= m_hand.get_value();

        return sum;
    }

    std::string Player::print_melds() const {
        std::string meldsStr;
        for (const auto& m : m_PlayedMelds) {
            meldsStr += m->to_string() + '\n';
        }

        return meldsStr;
    }

    unsigned char Player::get_hand_size() const {
        return m_hand.size();
    }

    bool Player::run_turn(GameState* gs) {
        return false;
    }

    std::shared_ptr<Card> Player::get_card(const uint8_t index) const {
        return m_hand.get_card(index);
    }

    std::shared_ptr<Player> Player::clone() const {
        return make_shared<Player>(*this);
    }

    void Player::close(){}
} // rummy::clients
