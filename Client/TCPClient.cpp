#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "RESPParser.hpp"

#define SERVER_IP "127.0.0.1"

/**
    @class TCPClient
    @brief Handles client-side network communication with the server.
    This client connects to a TCP server, encodes messages using RESP2,
    sends commands, and receives responses.
*/

class TCPClient {
private:
    int clientSocket;
    int serverPort;
    RESPParser parser;

public:
    /**
    * @brief Constructs a TCP client and creates a socket.
    * @param port The server port to connect to.
    */
    TCPClient(int port) {
        serverPort = port;
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
    }


    /**
     * @brief Connects the client to the TCP server.
     * @return True if the connection is successful, otherwise false.
     */
    bool connectToServer() {
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Connection to server failed");
            return false;
        }
        return true;
    }

    
    /**
     * @brief Sends a command to the server encoded in RESP2 format.
     * @param commandParts A vector containing command components.
     */
    void sendMessage(const std::vector<std::string>& commandParts) {
        std::string message = parser.encodeMessage(commandParts);
        send(clientSocket, message.c_str(), message.length(), 0);
    }


    /**
     * @brief Receives and returns a response from the server.
     * @return The server's response as a string.
     */
    std::string receiveMessage() {
        char buffer[1024];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0) return "";
        buffer[bytesRead] = '\0';
        return parser.decodeStringMessage(std::string(buffer));
    }

    /**
     * @brief Closes the connection to the server.
     */
    void closeConnection() {
        close(clientSocket);
    }
};

int main(int argc, char *argv[]) {
    int port = std::stoi(argv[1]);
    TCPClient client(port);
    if (!client.connectToServer()) return -1;
    
    std::string input;
    while (true) {
        std::cout << "client> ";
        std::getline(std::cin, input);
        if (input == "EXIT") break;
        
        std::istringstream iss(input);
        std::vector<std::string> commandParts;
        std::string word;
        while (iss >> word) {
            commandParts.push_back(word);
        }
        
        client.sendMessage(commandParts);
        std::cout << client.receiveMessage() << std::endl;
    }

    client.closeConnection();
    return 0;
}
