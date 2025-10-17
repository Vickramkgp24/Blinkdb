#include <iostream>
#include <vector>
#include <string>
#include "../part-a/LSMStorageEngine.hpp"

/**
    @class CommandHandler
    @brief Processes and executes RESP2 commands.
    This class is responsible for executing commands parsed from the RESP2 protocol,
    including interactions with the underlying storage engine.
*/
class CommandHandler {
private:
    LSMStorageEngine& storageEngine;

public:
    /**
        * @brief Constructs a CommandHandler with a reference to the storage engine.
        * @param engine A reference to the storage engine.
    */
    CommandHandler(LSMStorageEngine& engine) : storageEngine(engine) {}

    /**
     * @brief Executes a given command and returns a response.
     * @param commandParts A vector of strings representing the command components.
     * @return The response string formatted according to RESP2.
     */
    std::string executeCommand(const std::vector<std::string>& command) {
        if (command.empty()) return "-Error: Empty command\r\n";
        
        std::string operation = command[0];
        if (operation == "SET" && command.size() == 3) {
            storageEngine.set(command[1], command[2]);
            return "OK";
        } else if (operation == "GET" && command.size() == 2) {
            std::string value = storageEngine.get(command[1]);
            return value.empty() ? "" : value;
        } else if (operation == "DEL" && command.size() == 2) {
            storageEngine.del(command[1]);
            return "OK";
        } else {
            return "-Error: Unknown or malformed command\r\n";
        }
    }
};