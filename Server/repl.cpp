#include <iostream>
#include <fstream>
#include <sstream>
#include "LSMStorageEngine.hpp"

void startREPL(LSMStorageEngine& engine) {
    std::string command;
    while (true) {
        std::cout << "user> ";
        std::getline(std::cin, command);

        std::istringstream iss(command);
        std::string operation, key, value;
        iss >> operation;

        if (operation == "SET" || operation == "set") {
            iss >> key >> value;
            engine.set(key, value);
            std::cout << "OK\n";
        } else if (operation == "GET" || operation == "get") {
            iss >> key;
            std::string value;
            value = engine.get(key);
            if(value != "__TOMBSTONE__" && !value.empty()) {
                std::cout << value << "\n"; 
            } else {
                std::cout << "\n";
            }
        } else if (operation == "DEL" || operation == "del") {
            iss >> key;
            engine.del(key);
            std::cout << "OK\n";
        } else if (operation == "COMPACT" || operation == "compact") {
            engine.compact();
            std::cout << "OK\n";
        }  else if (operation == "EXIT" || operation == "exit") {
            break;
        } else {
            std::cout << "Unknown Command\n";
        }
    }
}

int main() {
    std::remove("wal.log");
    LSMStorageEngine engine;
    startREPL(engine);
    return 0;
}