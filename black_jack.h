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

    std::string getRank() const{
        switch(rank){
            case Rank::TWO: return "TWO";
            case Rank::THREE: return "THREE";
            case Rank::FOUR: return "FOUR";
            case Rank::FIVE: return "FIVE";
            case Rank::SIX: return "SIX";
            case Rank::SEVEN: return "SEVEN";
            case Rank::EIGHT: return "EIGHT";
            case Rank::NINE: return "NINE";
            case Rank::TEN: return "TEN";
            case Rank::JACK: return "JACK";
            case Rank::QUEEN: return "QUEEN";
            case Rank::KING: return "KING";
            case Rank::ACE: return "ACE";
        }
        return "NO CASE";
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

class Player {
public:
    int playerID;
    enum class Action { Start, Hit, Stand, Undefined, Quit};
    Action playerAction;
    std::vector<Card> cards;
    int bankroll;

public:
    Player(int numPlayer) : playerID(numPlayer) {}

    int getPlayerID() {
        return playerID;
    }

    void setPlayerAction(Action action) {
        playerAction = action;
    }

    Action deserializeAction(const std::string& actionStr) {
        if (actionStr == "hit") return Action::Hit;
        if (actionStr == "stand") return Action::Stand;
        if (actionStr == "quit") return Action::Quit;
        if(actionStr == "start") return Action::Start;
        return Action::Undefined;
    }

    void playerHit(Card card) {
        cards.push_back(card);
    }

    std::string getPlayerCards(){
        std::string gameState = "Player Cards: \n";
        for (const auto& card : cards) {

            gameState += "Card: "+card.getRank() + "\n";
        }
        return gameState;
    }

    int getHandValue() {
        int value = 0;
        int aces = 0;
        for (const auto& card : cards) {
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

    std::string determineOutcome(int dealerValue) {
        int playerValue = getHandValue();

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
};

class BlackjackGame {
public:
    std::vector<Player> players;
    std::vector<Card> dealerHand;

private:
    Deck deck;


public:
    BlackjackGame() {
        deck.reset();
    }

void dealerPlay(){
    int dealerValue = getHandValue(); 
    while (dealerValue < 17){
        dealerHand.push_back(deck.dealCard());
        dealerValue = getHandValue(); 
    }
}

    void dealInitialCards() {
        std::cout << "Dealing Dealer Cards." << std::endl; 
        dealerHand.push_back(deck.dealCard());
        dealerHand.push_back(deck.dealCard());
    }


void clearAll() {
    dealerHand.clear();
}

    void setPlayers(std::vector<Player>& playerSet){
    players = playerSet;
    }


    Card getCard(){
        return deck.dealCard();
    }

    std::string getDealerShowing(){
    return "Dealer Showing: "+dealerHand[0].getRank();
    }

    std::string serializeGameState() {
        std::string gameState = "Dealer Has: \n";
        for (const auto& card : dealerHand) {
            gameState += "Card: " + card.getRank()+ "\n";
        }
        return gameState;
    }

    bool isGameActive() {
        return getHandValue() < 21;
    }

int getHandValue() {
    if (dealerHand.empty()) {
        return 0; // Or whatever you want to return for an empty hand
    }

    int value = 0;
    int aces = 0;
    for (const auto& card : dealerHand) {
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
};