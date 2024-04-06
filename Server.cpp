#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include "thread.h"
#include "socketserver.h"
#include "black_jack.h"
#include <cstdlib>

using namespace Sync;

class SessionProcessor : public Thread { 
private:
    Socket& sessionSocket; 
    bool& isActive; 
    bool isFirstHand = true;
    std::vector<SessionProcessor*>& activeSessions;
    std::vector<Player> players;
    BlackjackGame game;

public:
    SessionProcessor(Socket& socket, bool& active, std::vector<SessionProcessor*>& sessions)
    : sessionSocket(socket), isActive(active), activeSessions(sessions), game() {
        std::string welcomeMessage = "Welcome to the Blackjack server! This game requires 2 players online to play";
        sessionSocket.Write(ByteArray(welcomeMessage));
    }

    virtual ~SessionProcessor() {
        terminationEvent.Wait();
    }

    Socket& GetSocket() {
        return sessionSocket;
    }

virtual long ThreadMain() override {
    try {
        int counter = 0;
        // To be implemented when synching - based on number of active sessions.
        // for (const auto& session : activeSessions) {
        //     players.push_back(Player(counter));
        //     counter++;
        // }
        players.push_back(Player(counter));
        bool isFirstHand = true;
        //Start of a blackjack game.
        while (players.size() > 0) {
            if (isFirstHand) {
                game.setPlayers(players);
                game.clearAll();
                game.dealInitialCards();
                game.dealerPlay();
                std::string dealerShowing = game.getDealerShowing() + "\n";
                sessionSocket.Write(ByteArray(dealerShowing));
                for (auto& player : players) {
                    player.playerHit(game.getCard());
                    player.playerHit(game.getCard());
                    std::string gameStart = player.getPlayerCards() + "\n";
                    sessionSocket.Write(ByteArray(gameStart));
                }
                isFirstHand = false;
            } else {
                for (auto& player : players) {
                    std::string playerToPlay = "Player ID" + std::to_string(player.getPlayerID()) + "'s turn to play \n";
                    sessionSocket.Write(ByteArray(playerToPlay));
                    bool isTurn = true;
                    while (isTurn) {
                        ByteArray clientData;
                        if (sessionSocket.Read(clientData)) {
                            std::string actionStr = clientData.ToString();
                            Player::Action action = player.deserializeAction(actionStr);
                            if(player.getHandValue() == 21){
                                std::string playerToPlay = "21 / Blackjack is hit! Autostand.";
                                sessionSocket.Write(ByteArray(playerToPlay));
                                isTurn = false;
                            }
                            if (action == Player::Action::Start) {
                                ;
                            } else if (action == Player::Action::Hit) {
                                player.playerHit(game.getCard());
                                std::string cards = player.getPlayerCards();
                                sessionSocket.Write(ByteArray(cards));
                                if(player.getHandValue()>20){
                                    isTurn = false;
                                }
                            } else if (action == Player::Action::Stand) {
                                isTurn = false;
                            } else if (action == Player::Action::Quit) {
                                break; // Break from the loop if the player quits
                            }
                        } else {
                            std::cerr << "Error" << std::endl;
                        }
                    }
                }
                    game.dealerPlay();
                    // Printing Results for each player
                    for (auto& player : players) {
                        std::string outcome = "Player "+std::to_string(player.getPlayerID()) + " result: " + game.serializeGameState() + "\n" + player.determineOutcome(game.getHandValue()) + "\n";
                        sessionSocket.Write(ByteArray(outcome));
                    }
            }
        }
    } catch (std::exception& e) {
        std::cerr << "Error " << e.what() << std::endl;
    }
    std::cout << "Session ended." << std::endl;
    return 0;
    }
};


class ServiceCoordinator : public Thread { 
private:
    SocketServer& networkServer; 
    bool isOperational;
    std::vector<SessionProcessor*> processingThreads; 

public:
    ServiceCoordinator(SocketServer& server)
    : networkServer(server), isOperational(true) {}

    virtual ~ServiceCoordinator(void) {
        for (auto& processor : processingThreads) {
            delete processor;
        }
        std::cout << "Service shutdown." << std::endl; 
    }

    virtual long ThreadMain() override {
        while (isOperational) {
            try {
                Socket* newConnection = new Socket(networkServer.Accept());
                SessionProcessor* newSession = new SessionProcessor(*newConnection, isOperational, processingThreads);
                processingThreads.push_back(newSession);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl; 
                isOperational = false; 
            }
        }
        return 0; 
    }

    void TerminateService() {
        isOperational = false;
        networkServer.Shutdown();
        for (auto& session : processingThreads) {
            try {
                session->GetSocket().Close();
            } catch (...) {
            }
        }
    }
};

int main(void) {
    SocketServer service(3000); 
    ServiceCoordinator coordinator(service);
    std::cout << "Press enter button to gracefully terminate" << std::endl; 
    std::cin.get(); 
    coordinator.TerminateService(); 
    return 0;
}

