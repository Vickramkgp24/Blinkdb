# Compiler
CXX = g++

# Executables
SERVER = TCPServer
CLIENT = TCPClient

# Source files
SERVER_SRC = src/TCPServer.cpp
CLIENT_SRC = src/TCPClient.cpp

# Default rule (build both)
all: $(SERVER) $(CLIENT)

# Compile the server
$(SERVER): $(SERVER_SRC)
	$(CXX) $(SERVER_SRC) -o $(SERVER)

# Compile the client
$(CLIENT): $(CLIENT_SRC)
	$(CXX) $(CLIENT_SRC) -o $(CLIENT)

# Run the server with port 9002
server: $(SERVER)
	./$(SERVER) 9002

# Run the client with port 9002
client: $(CLIENT)
	./$(CLIENT) 9002

# Clean compiled files
clean:
	rm -f $(SERVER) $(CLIENT)
