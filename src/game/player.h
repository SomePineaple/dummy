//
// Created by nj60 on 11/12/25.
//

#ifndef DUMMY_PLAYER_H
#define DUMMY_PLAYER_H
#include "meld.h"

namespace game {
    struct GameState;

    class Player {
    protected:
        Meld workingMeld;
        Pile hand;
        std::vector<std::shared_ptr<Meld>> playedMelds;

        void drawFromStock(GameState& gs);
        bool drawFromDiscard(GameState& gs, unsigned char numCards);
        bool playWorkingMeld(GameState& gs);
        void discard(GameState& gs, unsigned char cardNumber);
    public:
        unsigned short calcPoints();
        void printMelds() const;
        unsigned char getHandSize() const;
        virtual bool runTurn(GameState& gs);
    };

    class HumanPlayer : public Player {
        std::string name;
        void printGameState(const GameState& gs) const;
        bool askAndDiscard();
        void askAndAdd();
    public:
        HumanPlayer(const std::string& n) : name(n) {}

        bool runTurn(GameState& gs) override;
    };
} // game

#endif //DUMMY_PLAYER_H
