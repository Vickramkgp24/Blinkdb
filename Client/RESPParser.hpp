#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/**

    @class RESPParser
    @brief Parses and constructs RESP2 protocol messages.
    This class is responsible for decoding client commands formatted in the RESP2 protocol
    and encoding responses before sending them back to the client.
*/

class RESPParser {
    public:

        std::string encodeResponse(const std::string& response) {
            return "+" + response + "\r\n";
        }

        /**
            @brief Decodes a RESP2 formatted message into its component parts.
            @param message The raw RESP2 formatted string received from the client.
            @return A vector of strings representing the decoded command parts.
        */
        std::vector<std::string> decodeMessage(const std::string& message) {
            std::vector<std::string> result;
            std::istringstream stream(message);
            std::string line;
            while (std::getline(stream, line)) {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back(); // Remove trailing '\r'
                }
                if (!line.empty() && line[0] != '*' && line[0] != '$') {
                    result.push_back(line);
                }
            }
            return result;
        }

        std::string decodeStringMessage(const std::string& message) {
            if (message.empty()) return "";
            // only simple strings are expected from the server
            return message.substr(1, message.size() - 3); // Remove '+' prefix and '\r\n'
        }
    
        /**
            @brief Encodes a command message into RESP2 format.
            @param messageParts A vector of strings representing command components.
            @return The RESP2 formatted string.
        */
        std::string encodeMessage(const std::vector<std::string>& messageParts) {
            std::ostringstream stream;
            stream << "*" << messageParts.size() << "\r\n";
            for (const auto& part : messageParts) {
                stream << "$" << part.size() << "\r\n" << part << "\r\n";
            }
            return stream.str();
        }
};
