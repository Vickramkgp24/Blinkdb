#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include "RESPParser.hpp"
#include "CommandHandler.hpp"

#define MAX_EVENTS 10

/**
    @class EpollServer
    @brief Implements a TCP server using epoll for handling multiple clients.
*/

class EpollServer {
private:
    int serverSocket;
    int epollFd;
    std::unordered_map<int, std::string> clientBuffers;
    LSMStorageEngine storageEngine;
    RESPParser parser;
    CommandHandler commandHandler;

    /**
        @brief Sets a socket to non-blocking mode.
        @param sock The socket file descriptor.
    */
    void setNonBlocking(int sock) {
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    }

public:
    EpollServer(int port) : commandHandler(storageEngine) {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(serverSocket, SOMAXCONN) == -1) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        setNonBlocking(serverSocket);

        epollFd = epoll_create1(0);
        if (epollFd == -1) {
            perror("epoll_create1 failed");
            exit(EXIT_FAILURE);
        }

        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = serverSocket;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
            perror("epoll_ctl failed");
            exit(EXIT_FAILURE);
        }
    }

    /**
     * @brief Accepts a new client connection and registers it with epoll.
     */
    void acceptNewConnection() {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == -1) {
            perror("Accept failed");
            return;
        }
        setNonBlocking(clientSocket);

        epoll_event event{};
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = clientSocket;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);
    }
    /**
     * @brief Handles events for a client socket.
     * @param clientSocket The client socket file descriptor.
     * @param events The epoll event flags.
     */
    void handleClientEvent(int clientSocket, uint32_t events) {
        if (events & EPOLLIN) {
            char buffer[1024];
            int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
            if (bytesRead <= 0) {
                close(clientSocket);
                epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, nullptr);
                return;
            }
            buffer[bytesRead] = '\0';
            clientBuffers[clientSocket] += buffer;
            //std::cout<<"Received " <<clientBuffers[clientSocket]<<"\n";
            std::vector<std::string> command = parser.decodeMessage(clientBuffers[clientSocket]);
            if (!command.empty()) {
                std::string response = commandHandler.executeCommand(command);
                std::string encodedResponse = parser.encodeResponse(response);
                send(clientSocket, encodedResponse.c_str(), encodedResponse.size(), 0);
                clientBuffers[clientSocket].clear(); // Clear buffer after processing
            }
        }
    }
    /**
     * @brief Starts the epoll event loop to process client connections.
     */
    void start() {
        epoll_event events[MAX_EVENTS];
        while (true) {
            int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
            for (int i = 0; i < numEvents; ++i) {
                if (events[i].data.fd == serverSocket) {
                    acceptNewConnection();
                } else {
                    handleClientEvent(events[i].data.fd, events[i].events);
                }
            }
        }
    }
};

int main(int argc, char *argv[]) {
    std::ofstream::sync_with_stdio(false);

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    int port = std::stoi(argv[1]);
    EpollServer server(port);
    server.start();
    return 0;
}

