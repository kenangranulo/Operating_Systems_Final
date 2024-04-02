//Appropriate dependencies. 
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <cstdlib>
#include <string>

//Provides a scope
namespace Sync {

//Text modifier. Inherits from "thread.h"
class TextModifierClient : public Thread {  
private:
	//Socket
    Socket& serverConnection;  
	//Holds T or F determining if client is active
    bool& isActive;  
	//User input from terminal is held here
    std::string userInput;  

	//
    void interpretServerReply() {  
		//Server responses. Array will hold those values
        ByteArray serverReply;
		//Is there currently connection
        if (serverConnection.Read(serverReply) == 0) {
            isActive = false;
            std::cout << "Lost" << std::endl;
        } else {
            std::cout << serverReply.ToString() << std::endl;
        }
    }

public:
    TextModifierClient(Socket& connection, bool &running)
    : serverConnection(connection), isActive(running) {}

    virtual ~TextModifierClient() {}

	//When the client is active (true), then this while loop will iterazte
    virtual long ThreadMain() override {  
        while (isActive) {
			//User prompt
            std::cout << "Type here (If 'ok' is inputted, it will gracefully terminate the session): ";
			//Get the input from the cmd prompt. 
            std::getline(std::cin, userInput);
			//Terminate if user wants to end session
            if (userInput == "ok") {
                isActive = false;
                break;
            }

			//Transfer data to server
            serverConnection.Write(ByteArray(userInput));
            interpretServerReply();
        }

        return 0;
    }
};

} 

//Main function
int main() {
	//Application is running
    bool applicationRunning = true;
	//Connect to server
    Sync::Socket connection("127.0.0.1", 3000);
	//Instance of client with connection then opened
    Sync::TextModifierClient textModifier(connection, applicationRunning);
    connection.Open();

	//Iterate through while application is still running
    while (applicationRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

	//Close 
    connection.Close();
    return 0;
}