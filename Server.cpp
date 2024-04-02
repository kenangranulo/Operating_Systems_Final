/*
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include "thread.h"
#include "socketserver.h"
#include "black_jack.h" // Include the BlackjackGame class header
#include <cstdlib>

using namespace Sync;

class SessionProcessor : public Thread { 
private:
    Socket& sessionSocket; 
    bool& isActive; 
    std::vector<SessionProcessor*>& activeSessions;
    BlackjackGame game;

public:
    SessionProcessor(Socket& socket, bool& active, std::vector<SessionProcessor*>& sessions)
    : sessionSocket(socket), isActive(active), activeSessions(sessions), game(BlackjackGame()) {}

    virtual ~SessionProcessor() {
        terminationEvent.Wait();
    }

    Socket& GetSocket() {
        return sessionSocket;
    }

    virtual long ThreadMain() override {
        while (isActive) {
            try {
                ByteArray clientData;
                if (sessionSocket.Read(clientData)) {
                    std::string actionStr = clientData.ToString();
                    BlackjackGame::Action action = game.deserializeAction(actionStr);
                    
                    if (action == BlackjackGame::Action::Hit) {
                        game.playerHit();
                    } else if (action == BlackjackGame::Action::Stand) {
                        game.playerStand();
                    }
                    
                    std::string gameState = game.serializeGameState();
                    sessionSocket.Write(ByteArray(gameState));
                    
                    if (!game.isGameActive()) {
                        std::string outcome = game.determineOutcome();
                        sessionSocket.Write(ByteArray(outcome));
                        break;
                    }
                } else {
                    std::cerr << "Error" << std::endl;
                    break; 
                }
            } catch (std::exception& e) {
                std::cerr << "Error " << e.what() << std::endl; 
                break; 
            }
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
}
*/

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <exception>
#include "thread.h"
#include "socketserver.h"
#include "black_jack.h" // Include the BlackjackGame class header
#include <cstdlib>

using namespace Sync;

class SessionProcessor : public Thread { 
private:
    Socket& sessionSocket; 
    bool& isActive; 
    std::vector<SessionProcessor*>& activeSessions;
    BlackjackGame game;

public:
    SessionProcessor(Socket& socket, bool& active, std::vector<SessionProcessor*>& sessions)
    : sessionSocket(socket), isActive(active), activeSessions(sessions), game(BlackjackGame()) {
        // Immediately send a welcome message or initial game state to the client
        std::string welcomeMessage = "Welcome to the Blackjack server!";
        sessionSocket.Write(ByteArray(welcomeMessage));
    }

    virtual ~SessionProcessor() {
        terminationEvent.Wait();
    }

    Socket& GetSocket() {
        return sessionSocket;
    }

    virtual long ThreadMain() override {
        while (isActive) {
            try {
                ByteArray clientData;
                if (sessionSocket.Read(clientData)) {
                    std::string actionStr = clientData.ToString();
                    BlackjackGame::Action action = game.deserializeAction(actionStr);
                    
                    if (action == BlackjackGame::Action::Hit) {
                        game.playerHit();
                    } else if (action == BlackjackGame::Action::Stand) {
                        game.playerStand();
                    }
                    
                    std::string gameState = game.serializeGameState();
                    sessionSocket.Write(ByteArray(gameState));
                    
                    if (!game.isGameActive()) {
                        std::string outcome = game.determineOutcome();
                        sessionSocket.Write(ByteArray(outcome));
                        break;
                    }
                } else {
                    std::cerr << "Error" << std::endl;
                    break; 
                }
            } catch (std::exception& e) {
                std::cerr << "Error " << e.what() << std::endl; 
                break; 
            }
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
}
