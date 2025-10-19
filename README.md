# ⚡ Key-Value Storage Server with LSM-Tree and RESP Protocol

This repository contains a basic TCP Key-Value Storage Server implemented in C++. It utilizes an LSM (Log-Structured Merge) Tree for persistent data storage, handles concurrent connections using the epoll mechanism, and communicates using a simplified version of the RESP (REdis Serialization Protocol).

## ✨ Features

- **LSM Storage Engine:** Implements a simple LSM-Tree structure with an in-memory MemTable, a Write-Ahead Log (WAL), and a persistent SSTable.  
- **Persistence:** Data is saved to `wal.log` and flushed to `sstable.txt`.  
- **Recovery:** Automatically restores state from the WAL upon startup.  
- **Compaction:** Includes a `compact()` method to merge SSTables and remove deleted or outdated entries.  
- **Thread Safety:** Uses `std::mutex` to protect shared state during concurrent operations.  
- **TCP Server:** Built with the epoll I/O event notification facility for efficient handling of multiple client connections.  
- **RESP Protocol:** Implements a basic `RESPParser` for encoding and decoding messages, compatible with tools like `redis-benchmark`.  
- **Command Line Interface:** Includes a simple `repl.cpp` for local, non-networked testing of the LSM storage engine.

## 📁 Project Structure

| File Name             | Description                                              |
|-----------------------|----------------------------------------------------------|
| `TCPServer.cpp`       | Main server implementation using epoll for concurrent I/O. |
| `TCPClient.cpp`       | Simple command-line client for interacting with the server. |
| `repl.cpp`            | Standalone REPL for local testing of the LSMStorageEngine. |
| `LSMStorageEngine.hpp`| Core LSM-Tree-based key-value storage engine.             |
| `AbstractStorageEngine.hpp` | Defines the abstract interface for the storage engine.  |
| `CommandHandler.hpp`  | Parses and executes client commands against the storage engine. |
| `RESPParser.hpp`      | Implements RESP2 encoding/decoding logic.                 |
| `evaluate.sh`         | Shell script for running benchmarks using redis-benchmark. |

## 🛠️ Building the Project (Manual Compilation)

You’ll need a C++17-compatible compiler (like `g++`) installed on your system.

### 1️⃣ Compile the Server
Compile the epoll-based server manually:

g++ -std=c++17 -pthread TCPServer.cpp -o server

This will create an executable named `server`.

### 2️⃣ Compile the Client
Compile the client for interacting with the server:

g++ -std=c++17 TCPClient.cpp -o client

This will create an executable named `client`.

### 3️⃣ Compile the Local REPL (optional)
If you want to test the LSM storage engine locally without networking:

g++ -std=c++17 repl.cpp -o repl

## 🚀 Running the System

### 🖥️ Start the Server
Run the server on a desired port (e.g., `9002`):

./server 9002

The server will start and wait for client connections.

### 💻 Start the Client
Open a new terminal and run:

./client 9002

You can now enter commands such as:

SET key value<br>
GET key<br>
DEL key<br>
COMPACT<br>
EXIT

The client will communicate with the server using the RESP protocol.

### 🧪 Local REPL (Without Network)
If you compiled the REPL version, you can test the engine directly:

./repl

Available commands:

set key value<br>
get key<br>
del key<br>
compact<br>
exit

## 🧩 Supported Commands

| Command | Usage         | Description                                      |
|---------|---------------|------------------------------------------------|
| SET     | `SET key value` | Sets a key to a value.                          |
| GET     | `GET key`     | Retrieves the value of a key. Returns empty if deleted or missing. |
| DEL     | `DEL key`     | Marks a key as deleted using a tombstone.      |
| COMPACT | `COMPACT`     | Cleans up SSTables, removing duplicates and tombstones. |

## ⏱️ Benchmarking

You can benchmark the server using the included `evaluate.sh` script with `redis-benchmark`.

### Prerequisites

- The server must be running (e.g., `./server 9002`)  
- The `redis-benchmark` tool must be installed (`sudo apt install redis-tools`)  
- Ensure the server port matches the script (default: `9002`)

### Run Benchmark

./evaluate.sh

The script will run `redis-benchmark` for multiple combinations of requests and clients. For example:

| Requests | Clients | Output File           |
|----------|---------|----------------------|
| 10,000   | 10      | result_10000_10.txt  |
| 10,000   | 100     | result_10000_100.txt |
| 10,000   | 1000    | result_10000_1000.txt|
| 100,000  | 10      | result_100000_10.txt |
| ...      | ...     | ...                  |

Results will be saved in corresponding `result_*.txt` files.

## 📊 Example Performance Results

| Operation | Requests | Clients | Throughput (req/s) | Avg Latency (ms) |
|-----------|----------|---------|--------------------|------------------|
| SET       | 100,000  | 1000    | 129,701            | 3.96             |
| GET       | 100,000  | 1000    | 156,006            | 3.22             |

✅ Achieves over 150K GET/s and 130K SET/s in single-threaded mode, using epoll and persistent LSM-based storage.

## 🧹 Cleaning Up

To remove executables and persistent files:

rm -f server client repl
rm -f wal.log sstable.txt

---

Feel free to customize any part of this README further to match your exact use or style.
