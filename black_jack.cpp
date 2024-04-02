/*
#include <iostream>
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

void printHand(const std::vector<Card>& hand) {
    for (const auto& card : hand) {
        std::string rank, suit;
        switch (card.rank) {
            case Rank::TWO: rank = "2"; break;
            case Rank::THREE: rank = "3"; break;
            case Rank::FOUR: rank = "4"; break;
            case Rank::FIVE: rank = "5"; break;
            case Rank::SIX: rank = "6"; break;
            case Rank::SEVEN: rank = "7"; break;
            case Rank::EIGHT: rank = "8"; break;
            case Rank::NINE: rank = "9"; break;
            case Rank::TEN: rank = "10"; break;
            case Rank::JACK: rank = "Jack"; break;
            case Rank::QUEEN: rank = "Queen"; break;
            case Rank::KING: rank = "King"; break;
            case Rank::ACE: rank = "Ace"; break;
            default: rank = "Unknown";
        }

        switch (card.suit) {
            case Suit::HEARTS: suit = "Hearts"; break;
            case Suit::DIAMONDS: suit = "Diamonds"; break;
            case Suit::CLUBS: suit = "Clubs"; break;
            case Suit::SPADES: suit = "Spades"; break;
            default: suit = "Unknown";
        }

        std::cout << rank << " of " << suit << std::endl;
    }
}

int main() {
    Deck deck;
    std::vector<Card> playerHand, dealerHand;
    char choice;

    // Initial deal
    playerHand.push_back(deck.dealCard());
    dealerHand.push_back(deck.dealCard());
    playerHand.push_back(deck.dealCard());
    dealerHand.push_back(deck.dealCard());

    // Player's turn
    while (true) {
        std::cout << "Your hand:" << std::endl;
        printHand(playerHand);
        std::cout << "Total value: " << getHandValue(playerHand) << std::endl;

        if (getHandValue(playerHand) >= 21) break;

        std::cout << "Do you want to hit (h) or stand (s)? ";
        std::cin >> choice;

        if (choice == 'h') {
            playerHand.push_back(deck.dealCard());
        } else {
            break;
        }
    }

    // Dealer's turn
    while (getHandValue(dealerHand) < 17) {
        dealerHand.push_back(deck.dealCard());
    }

    std::cout << "Dealer's hand:" << std::endl;
    printHand(dealerHand);
    std::cout << "Total value: " << getHandValue(dealerHand) << std::endl;

       // Determine winner
    int playerValue = getHandValue(playerHand);
    int dealerValue = getHandValue(dealerHand);

    if (playerValue > 21) {
        std::cout << "You bust! Dealer wins." << std::endl;
    } else if (dealerValue > 21) {
        std::cout << "Dealer busts! You win!" << std::endl;
    } else if (playerValue > dealerValue) {
        std::cout << "You win!" << std::endl;
    } else if (playerValue < dealerValue) {
        std::cout << "Dealer wins." << std::endl;
    } else {
        std::cout << "It's a tie!" << std::endl;
    }

    return 0;
}
*/

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
