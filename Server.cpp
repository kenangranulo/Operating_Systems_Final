// Importing necessary libraries
#include "thread.h"
#include "socketserver.h"
#include "Semaphore.h"
#include <stdlib.h>
#include <time.h>
#include <tuple>
#include <list>
#include <vector>
#include <algorithm>
#include <jsoncpp/json/json.h>

using namespace Sync;

// Keeping global reference to server (helps with deconstructing later)
SocketServer *server;
int playerID = 0;
const int MAX_PLAYERS = 4;
std::vector<Socket> playerSockets;

// Creating a card struct; this will hold the suit and card number in one
struct Card
{
	std::string suit;
	int num;

	// Formats it to a nice string
	std::string toString() const
	{
		std::string str;
		switch (num)
		{
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
			str = std::to_string(num);
			break;
		}
		return str + suit;
	}
};

// Function to get a random number
int randomNum(int min, int max)
{
	return (rand() % (max - min + 1) + min);
}

// Function to get a random card 
Card getRandomCard()
{
	Card card;

	std::string suits[] = {"D", "H", "S", "C"};
	card.suit = suits[randomNum(0, 3)];
	card.num = randomNum(1, 13);

	return card;
}

// Function that deals with how cards are dealt
std::vector<Card> getCards(int numberOfCards, std::vector<Card> dealtCards)
{
	std::vector<Card> cards;

	// Deals new cards while ensuring that the random card doesn't already have 6 of them (6 decks in blackjack)
	for (int i = 0; i < numberOfCards; i++)
	{
		int count = 6;
		Card newCard;
		
		while (count == 6)
		{
			newCard = getRandomCard();
			count = 0;
			for (int i = 0; i < dealtCards.size(); i++)
			{
				if (dealtCards[i].num == newCard.num && dealtCards[i].suit == newCard.suit)
				{
					count++;
				}
			}
		}

		// Add card to dealt cards
		cards.push_back(newCard);
		dealtCards.push_back(newCard);
	}

	return cards;
}

// Function to convert Card array to JSON
Json::Value from(std::vector<Card> arr)
{
	Json::Value convertedArr(Json::arrayValue);

	for (Card inst : arr)
	{
		convertedArr.append(inst.toString());
	}

	return convertedArr;
}

// Function to get the card sum; this function will return two values if there is an ace
// this is given that if there is an ace, it can either be used as a 1 or 10
std::vector<int> cardSum(std::vector<Card> cards) {
	std::vector<int> total = {0, 0};

	bool hasAce = false;

	for(Card card : cards) {
		if (card.num == 1) {
			hasAce = true;
			total[1] = total[0] + 10;
		}

		// Card number could be 11, 12, or 13 (J, Q, K); they're all used as 10s so interpret them as such
		total[0] += std::min(card.num, 10);
		if (hasAce) {
			total[1] += std::min(card.num, 10);
		}
	}

	return total;
}

// Method to get the card sum string; if theres two numbers, send them back with a slash in between
std::string formatCardSum(std::vector<int> cardSum) {
	return cardSum[1] != 0 ? (std::to_string(cardSum[0]) + "/" + std::to_string(cardSum[1])) : 
			std::to_string(cardSum[0]);
}

// Function to check if a card array is busted (could be used by player or dealers)
bool isBusted(std::vector<Card> cards) {
	std::vector<int> total = cardSum(cards);

	// Depending on if they have an ace
	if (total[1] == 0) {
		return total[0] > 21;
	} else {
		return total[0] > 21 && total[1] > 21;
	}
}

// Function to check if they're done their turn (that's if they've reached their max) or busted
bool doneTurn(std::vector<Card> cards, int max) {
	std::vector<int> total = cardSum(cards);

	return total[0] == max || total[1] == max || 
	(total[1] == 0 && total[0] > max) || 
	*min_element(total.begin(), total.end()) > max;
}

// Function to get the higher total that is less than 21
int getHigherTotal(std::vector<Card> cards) {
	std::vector<int> total = cardSum(cards);

	int actSum = total[0] > total[1] && total[0] <= 21 ? total[0] : total[1];

	return actSum;
}

// Struct that defines the players information
struct Player {
    int id;
    int seat;
    int bet;
    std::vector<Card> cards;
    int balance;
    int isActive;
	bool isDoneTurn;
    int hasWon;
    
    Player(int p_id, int p_seat, int p_bet, std::vector<Card> p_cards, 
			int p_balance, int p_isActive, bool p_hasWon)
        : id(p_id), seat(p_seat), bet(p_bet), cards(p_cards),
          balance(p_balance), isActive(p_isActive), hasWon(p_hasWon), isDoneTurn(false)
    {}

	// Function to get the JSON version of the player data
	Json::Value toJson()
	{
		Json::Value converted(Json::objectValue);

		converted["bet"] = bet;
		converted["cards"] = from(cards);
		converted["balance"] = balance;
		converted["isActive"] = isActive;
		converted["cardSum"] = formatCardSum(cardSum(cards));
		converted["isBusted"] = isBusted(cards);
		converted["hasWon"] = hasWon;

		return converted;
	}
};

// Struct to define the structure of the game
struct Game {
	int gameID;
	std::vector<Player*> players;
	std::vector<Card> dealtCards;
	std::vector<Card> dealerCards;
	int currentState;
	int timeRemaining;
	int currentSeatPlaying;
	Json::Value gameState;


	Game(int id, int state, int timeRemaining, int currentSeat): gameID(id), currentState(state), 
	timeRemaining(timeRemaining), currentSeatPlaying(currentSeat)
	{}

	// Function to get the number of players
	int getNumberOfPlayers() {
		return players.size();
	}

	int getNumberOfActivePlayers() {
		int numberOfPlayers = 0;

		for(int i = 0; i < players.size(); i++) {
			numberOfPlayers += players[i]->isActive == 0 ? 1 : 0;
		}

		return numberOfPlayers;
	}

	void updateSeats() {
		for(int i = 0; i < this->getNumberOfPlayers(); i++) {
			this->players[i]->seat = i;
		}
	}

	// Function to add a player to the game; this needs to lock as the Dealer Thread uses it too
	void addPlayer(Player * newPlayer) {
		Semaphore mutex("mutex");

		mutex.Wait();

		this->players.push_back(newPlayer);
		
		// Update the seats based on the new player joining
		updateSeats();

		mutex.Signal();
		std::cout << "Seat: " << this->players[0]->seat << std::endl;
	}

	void removePlayer(int idToRemove) {

		// Find the player and remove them from the array
		for (auto it = this->players.begin(); it != this->players.end(); ++it) {
			if ((*it)->id == idToRemove) {
				this->players.erase(it);
				break;
			}
		}
		
		updateSeats();
	}
	
};

std::vector<Game*> games;

// Function to convert the player array to JSON
Json::Value from(std::vector<Player*> arr) {
	Json::Value convertedArr(Json::objectValue);

	for(Player * inst : arr) {
		convertedArr[std::to_string(inst->id)] = inst->toJson(); 
	}

	return convertedArr;
}

// DealerThread is initiated per game and manages the braodcasting of game state and updating the state when necessary
class DealerThread : public Thread{
	private:
		int TIME_BETWEEN_REFRESHES;
		int idx;
		static int timeToTerminate;
		static bool isTimerActive;

	public:
		DealerThread(int gameIdx):Thread(1000),TIME_BETWEEN_REFRESHES(1),idx(gameIdx) {
			Thread::Start();
		}

		// Before broadcasting to all connected players, dealer updates the JSON game state
		void updateGameState() {
			games[idx]->gameState["dealerCards"] = from(games[idx]->dealerCards);
			games[idx]->gameState["hasDealerBusted"] = isBusted(games[idx]->dealerCards);
			games[idx]->gameState["timeRemaining"] = games[idx]->timeRemaining;
			games[idx]->gameState["dealerSum"] = formatCardSum(cardSum(games[idx]->dealerCards));
			games[idx]->gameState["players"] = from(games[idx]->players);
		}

		virtual long ThreadMain(void) override{
			while(playerSockets.size()<2){
                sleep(1); // Wait for a second before checking again
            }
			Semaphore broadcast("broadcast");
			Semaphore mutex("mutex");
			while(true)
			{
				// Thread sleeps for a predefined amount in between game updates (usually a second)
				sleep(TIME_BETWEEN_REFRESHES);

				// Waits until thread can safely edit the games object
				mutex.Wait();
				
				// Tiem remaining refers to the time left for the current state of play (i.e. betting, playing, etc.)
				games[idx]->timeRemaining -= games[idx]->getNumberOfPlayers() > 0 ? TIME_BETWEEN_REFRESHES : 0;

				// Execut game state change if time ran out
				bool shouldUpdateSeat = (games[idx]->currentState == 1 && games[idx]->currentSeatPlaying < games[idx]->getNumberOfActivePlayers() && games[idx]->players[games[idx]->currentSeatPlaying]->isDoneTurn);

				if(games[idx]->getNumberOfActivePlayers() < 2){
					if(!isTimerActive){
						std::cout << "Number of active players is less than 2. Starting 30 second timer..." << std::endl;
						timeToTerminate = 30;
						isTimerActive = true;
					} else {
						timeToTerminate -= TIME_BETWEEN_REFRESHES;
						std::cout << "Time to terminate: " << timeToTerminate << " seconds" <<std::endl;
						if(timeToTerminate <= 0) {
							 std::cout << "No player joined in time. Terminating server." << std::endl;
                        	delete server;
                        	exit(0);
						}
					}
				} else {
					isTimerActive = false;
				}

				if (games[idx]->timeRemaining <= 0 || shouldUpdateSeat) {	
					// If game state is currently in play
					if (shouldUpdateSeat) {
						// Increment to next player
						games[idx]->currentSeatPlaying++;
					}

					// If game state is currently in betting state
					if (games[idx]->currentState == 0) {
						if (games[idx]->getNumberOfPlayers() > 0) {

							// Transition to playing state
							games[idx]->currentState = 1;
							if (games[idx]->dealtCards.size() >= 156)
							{
								games[idx]->dealtCards.clear();
							}

							// Get dealer's cards
							games[idx]->dealerCards = getCards(2, games[idx]->dealtCards);

							// Activate all necessary players and remove players that have been de-activated
							for (int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
								if (games[idx]->players[i]->isActive == 1)
									games[idx]->players[i]->isActive = 0;

								if (games[idx]->players[i]->isActive == 0)
									games[idx]->players[i]->cards = getCards(2, games[idx]->dealtCards);
								else if (games[idx]->players[i]->isActive == 2)
									games[idx]->removePlayer(games[idx]->players[i]->id);
							}
							games[idx]->currentSeatPlaying = 0;
						} else {
							// Reset if no players
							games[idx]->currentSeatPlaying = 0;
							games[idx]->currentState = 0;
							games[idx]->dealerCards = {};							
						}
					}
					// This branch executes once all players have played their turn
					else if (games[idx]->currentState == 1 && games[idx]->currentSeatPlaying >= games[idx]->getNumberOfPlayers()) {
						while(!doneTurn(games[idx]->dealerCards, 17)) {
							games[idx]->dealerCards.push_back(getRandomCard());
						}

						bool hasDealerBusted = isBusted(games[idx]->dealerCards);

						for (int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
							bool hasPlayerBusted = isBusted(games[idx]->players[i]->cards);

							// Depending on the cards of the dealer and each of the player, the winner is decided in the following if blocks
							if (!hasPlayerBusted && ((hasDealerBusted) || (getHigherTotal(games[idx]->players[i]->cards) > getHigherTotal(games[idx]->dealerCards)))) { // winner
								games[idx]->players[i]->hasWon = 1;
								games[idx]->players[i]->balance += games[idx]->players[i]->bet * 2;
							}
							else if(hasPlayerBusted || (!hasDealerBusted && (getHigherTotal(games[idx]->players[i]->cards) < getHigherTotal(games[idx]->dealerCards)))) { // loser
								games[idx]->players[i]->hasWon = 0;
								games[idx]->players[i]->balance -= games[idx]->players[i]->bet;
							} else {
								games[idx]->players[i]->hasWon = 2;
								games[idx]->players[i]->balance += games[idx]->players[i]->bet;
							}
						}
						games[idx]->currentState = 2;
					} else if (games[idx]->currentState == 2) {
						// If game has ended, reset game
						games[idx]->currentSeatPlaying = 0;
						games[idx]->currentState = 0;
						games[idx]->dealerCards = {};

						for(int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
							if(games[idx]->players[i]->balance <= 0) {
								games[idx]->removePlayer(i);
							}
							games[idx]->players[i]->isDoneTurn = false;
							games[idx]->players[i]->bet = 0;
							games[idx]->players[i]->cards = {};							
						}
					}

					games[idx]->timeRemaining = 10;
				}

				updateGameState();

				mutex.Signal();
			
				for(int i = 0; i < games[idx]->getNumberOfPlayers(); i++) {
					// Signals to each player to write to the socket for the frontend to receive game state
					broadcast.Signal();
					
				}
			}
	
		}
};

int DealerThread::timeToTerminate = 30;
bool DealerThread::isTimerActive = false;


// PlayerReader thread is responsible for broadcasting the gamestate to the client of each player
class PlayerReader : public Thread
{
	private:
		int playerID;
		int idx;
	public:
		Socket socket;
		
		PlayerReader(Socket & sock, int playerID, int gameIdx):Thread(1000),socket(sock),idx(gameIdx){
			this->playerID = playerID;
			this->idx = gameIdx;
			Thread::Start();
		}
		
		virtual long ThreadMain(void) override{
			while(playerSockets.size()<2){
                sleep(1); // Wait for a second before checking again
            }
			
			std::cout << "A player reader thread has started." << std::endl;
			
			Semaphore broadcast("broadcast");

			Json::Value initalBroadcast(Json::objectValue);
			initalBroadcast["playerID"] = this->playerID;

			ByteArray responseBuffer(initalBroadcast.toStyledString());
			socket.Write(responseBuffer);
			
			while(true)
			{
				sleep(1);
				if(!socket.isOpen()) {
					break;
				}
				ByteArray responseBuffer(games[idx]->gameState.toStyledString());
				socket.Write(responseBuffer);
			}		
		}
};

// PlayerWriter thread is responsible for waiting to receive data from the client, and updates the gamestate accordingly
class PlayerWriter : public Thread
{
	private:
		int playerID;
		int idx;

	public:
		Socket socket;
		Player data;

		PlayerWriter(Socket &sock, int playerID, int gameIdx)
			: Thread(1000), socket(sock),data(playerID, 0, 0, {}, 200, 1, false), idx(gameIdx)
		{
			Thread::Start();
		}
		
		virtual long ThreadMain(void) override{
			while(playerSockets.size()<2){
                sleep(1); // Wait for a second before checking again
            }			
			std::cout << "A player writer thread has started on Game #" << std::to_string(games[idx]->gameID) << std::endl;
			
			Semaphore mutex("mutex");
			
			Player * data_ptr = &data;
			games[idx]->addPlayer(data_ptr);			
			while (true)
			{
				ByteArray * buffer = new ByteArray();
				try {
					if (socket.Read(*buffer) == 0){
						throw std::string("Socket closed by client");
					}

					mutex.Wait();

					std::string req = (*buffer).ToString();
					Json::Value playerAction(Json::objectValue);
					Json::Reader reader;
					reader.parse(req, playerAction);

					if (playerAction["type"].asString() == "DONE")
					{
						std::cout << "Player-" << std::to_string(data.id) << " left Game #" << std::to_string(games[idx]->gameID) << std::endl;
						data.isActive = 2; // Set player status to inactive
						games[idx]->removePlayer(data.id); // Remove player from the game
						updateGameState(); // Update game state to reflect the changes
						//socket.Close(); // Close the socket
						mutex.Signal();
						break; // Exit the loop
					}

					// This if block executes when the client indicates that they are hitting or standing
					if (playerAction["type"].asString() == "TURN")
					{
						std::string action = playerAction["action"].asString();
							std::cout << action << std::endl;
							if (action == "HIT") {
								// If player hits, append new card to player
								data.cards.push_back(getRandomCard());
								
								// If sum of player's cards is over 21, go ot next player
								data.isDoneTurn = doneTurn(data.cards, 21);
							} else {
								// If player stands, go to next player
								data.isDoneTurn = true;
							}
						} else {
							// If player indicates that they are betting, adjust bet amount
							int betAmn = playerAction["betAmount"].asInt();
							data.balance -= betAmn; 
							data.bet = betAmn;
						}

					mutex.Signal();
				}
				catch (std::string err){
					std::cout << err << std::endl;
					break;
				}
			}
		}

		void updateGameState(){
			games[idx]->gameState["dealerCards"] = from(games[idx]->dealerCards);
    		games[idx]->gameState["hasDealerBusted"] = isBusted(games[idx]->dealerCards);
			games[idx]->gameState["timeRemaining"] = games[idx]->timeRemaining;
			games[idx]->gameState["dealerSum"] = formatCardSum(cardSum(games[idx]->dealerCards));
			games[idx]->gameState["players"] = from(games[idx]->players);
			games[idx]->gameState["numberOfActivePlayers"] = games[idx]->getNumberOfActivePlayers(); // Update the number of active players
		}
	};


// Termination
class ServerTerminationInput : public Thread
{
	public:
		std::string input;

		ServerTerminationInput() : Thread(1000)
		{
			Thread::Start();
		}

		virtual long ThreadMain(void) override{
			while (true) {
				std::cin >> input;
				if (input == "done") {
					Json::Value terminationMsg;
    				terminationMsg["type"] = "server_termination";
    				terminationMsg["message"] = "Server is terminating. Connection will be closed.";
        				for(auto it : playerSockets){
            				ByteArray responseBuffer(terminationMsg.toStyledString());
            				it.Write(responseBuffer);
							std::cout << "Sending Termination...." << std::endl;
        				}
					std::cout << "Done Server...." << std::endl;
					delete server;
					exit(0);
				}
			}
		}
};

int main(int argc, char* argv[])
{
    std::cout << "BlackJack Server" << std::endl;

    int port = argc >= 2 ? std::stoi(argv[1]) : 1500;

	// Server starts listening on the defined port number
	server = new SocketServer(port);

	int curGameID = 0;

	// Initiates the first game of blackjack
	Game firstGame(curGameID++, 0, 10, 0);
	games.push_back(&firstGame);
	DealerThread dealer(0);
	std::cout << "Port: " << port << std::endl;

	// Iniitalize semaphores
	Semaphore mutex("mutex", 1, true);
	Semaphore broadcast("broadcast", 0, true);

	bool hasJoined = false;

	ServerTerminationInput * terminationInput = new ServerTerminationInput();

	srand(time(0));

	while (true)
	{
		try
		{
			// Wait for joining clients
			Socket sock = server->Accept();
			playerSockets.push_back(sock);
			int gameID;
			hasJoined = false;
			//Waiting for a bit.
			for(int c=0; c<111111; c++){
				;
			}
			// This for loop determines index of game that is not full
			for(int i = 0; i < games.size(); i++) {
				if (games[i]->getNumberOfPlayers() < MAX_PLAYERS) {
					gameID = i;
					hasJoined = true;
					break;
				}
			}

			// If all games are full, new game/transaction is created
			if (!hasJoined) {
				std::cout << "All games were full. Creating a new game";
				Game newGame(curGameID++, 0, 10, 0);
				games.push_back(&newGame);

				gameID = curGameID - 1;
				while(games[gameID] == nullptr) {
					std::cout << ".";
				}

				std::cout << "Test" << std::endl;
				DealerThread * dealer = new DealerThread(gameID);
			}

			playerID++;
			std::cout << "GameID: " << gameID << std::endl;

			// Start the player threads
			PlayerReader * reader = new PlayerReader(sock, playerID, gameID);
			PlayerWriter * writer = new PlayerWriter(sock, playerID, gameID);
    	} catch (std::string err) {
    		if (err == "Unexpected error in the server") {
    			std::cout << "Server is terminated" << std::endl;
    			break;
    		}
    	}
    }    
}