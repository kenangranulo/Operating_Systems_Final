#pragma once

#include <vector>
#include <random>
#include <ctime>
#include <algorithm>
#include <string>

enum class Rank { TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, ACE };
enum class Suit { HEARTS, DIAMONDS, CLUBS, SPADES };

struct Card {
    Rank rank;
    Suit suit;

    int getValue() const {
        if (rank == Rank::JACK || rank == Rank::QUEEN || rank == Rank::KING) {
            return 10;
        }
        if (rank == Rank::ACE) {
            return 11;
        }
        return static_cast<int>(rank) + 2;
    }
};

class Deck {
private:
    std::vector<Card> cards;
    size_t currentCard = 0;

public:
    Deck() {
        reset();
    }

    void reset() {
        cards.clear();
        for (int suit = 0; suit < 4; ++suit) {
            for (int rank = 0; rank < 13; ++rank) {
                cards.push_back(Card{static_cast<Rank>(rank), static_cast<Suit>(suit)});
            }
        }
        currentCard = 0;
        std::shuffle(cards.begin(), cards.end(), std::default_random_engine(static_cast<unsigned int>(time(nullptr))));
    }

    Card dealCard() {
        return cards[currentCard++];
    }
};

class BlackjackGame {
public:
    enum class Action { Hit, Stand, Undefined };

private:
    Deck deck;
    std::vector<Card> playerHand, dealerHand;

    void dealInitialCards() {
        playerHand.push_back(deck.dealCard());
        dealerHand.push_back(deck.dealCard());
        playerHand.push_back(deck.dealCard());
        dealerHand.push_back(deck.dealCard());
    }

    int getHandValue(const std::vector<Card>& hand) {
        int value = 0;
        int aces = 0;
        for (const auto& card : hand) {
            value += card.getValue();
            if (card.rank == Rank::ACE) {
                ++aces;
            }
        }

        while (value > 21 && aces > 0) {
            value -= 10;
            --aces;
        }

        return value;
    }

public:
    BlackjackGame() {
        deck.reset();
        dealInitialCards();
    }

    std::string serializeGameState() {
        std::string gameState;
        for (const auto& card : playerHand) {
            gameState += std::to_string(static_cast<int>(card.rank)) + "," + std::to_string(static_cast<int>(card.suit)) + ";";
        }
        gameState += "|";
        for (const auto& card : dealerHand) {
            gameState += std::to_string(static_cast<int>(card.rank)) + "," + std::to_string(static_cast<int>(card.suit)) + ";";
        }
        return gameState;
    }

    Action deserializeAction(const std::string& actionStr) {
        if (actionStr == "hit") return Action::Hit;
        if (actionStr == "stand") return Action::Stand;
        return Action::Undefined;
    }

    void playerHit() {
        playerHand.push_back(deck.dealCard());
    }

    void playerStand() {
        while (getHandValue(dealerHand) < 17) {
            dealerHand.push_back(deck.dealCard());
        }
    }

    std::string determineOutcome() {
        int playerValue = getHandValue(playerHand);
        int dealerValue = getHandValue(dealerHand);

        if (playerValue > 21) {
            return "Player busts; Dealer wins.";
        } else if (dealerValue > 21) {
            return "Dealer busts; Player wins.";
        } else if (playerValue > dealerValue) {
            return "Player wins.";
        } else if (playerValue < dealerValue) {
            return "Dealer wins.";
        } else {
            return "It's a tie.";
        }
    }

    bool isGameActive() {
        return getHandValue(playerHand) < 21;
    }
};
