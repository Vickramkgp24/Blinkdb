#include <iostream>
#include <fstream>
#include <mutex>
#include <map>
#include <unordered_map>
#include <functional>
#include "AbstractStorageEngine.hpp"

class LSMStorageEngine : public StorageEngine {
private:
    std::map<std::string, std::string> memTable;
    std::ofstream walStream;  ///< Persistent WAL file
    std::mutex storageMutex; ///< Mutex for thread safety
    std::string walFile = "wal.log";
    std::string sstableFile = "sstable.txt";
    const size_t MEMTABLE_THRESHOLD = 2; // small for testing
    const std::string TOMBSTONE = "__TOMBSTONE__"; ///< Marker for deleted Keys
    std::unordered_map<std::string, std::streampos> index; ///< Index for fast lookups

    // NOTE: This function MUST be called while the caller already holds storageMutex.
    // It deliberately does NOT lock storageMutex to avoid double-lock/deadlock.
    void flushToDisk() {
        std::ofstream sstable(sstableFile, std::ios::app);
        if(!sstable) {
            std::cerr << "Error Opening SSTable File!\n";
            return;
        }

        for (const auto& pair : memTable) {
            index[pair.first] = sstable.tellp(); // store position before writing
            sstable << pair.first << " " << pair.second << "\n";
        }

        sstable.close();
        memTable.clear();

        // Truncate and reopen WAL safely
        walStream.close();
        std::ofstream truncFile(walFile, std::ios::trunc);
        truncFile.close();

        walStream.open(walFile, std::ios::app);
        if (!walStream.is_open()) {
            std::cerr << "Error reopening WAL after flush!\n";
        }
    }

    void writeAheadLog(const std::string& key, const std::string& value) {
        if(walStream.is_open()) {
            walStream << key << " " << value << "\n";
            walStream.flush();
        } else {
            std::cerr << "WAL is not open -- dropping log for: " << key << "\n";
        }
    }

    void buildIndex() {
        index.clear();
        std::ifstream sstable(sstableFile);
        if(!sstable) {
            return;
        }
        std::string key, value;
        std::streampos pos;
        while (sstable) {
            pos = sstable.tellg();
            if (sstable >> key >> value) {
                index[key] = pos;
            }
        }
        sstable.close();
    }

public:
    LSMStorageEngine() {
        walStream.open(walFile, std::ios::app);
        if(!walStream) {
            std::cerr << "Error opening WAL File!\n";
        }
        recover();
        buildIndex();
    }

    ~LSMStorageEngine() {
        if(walStream.is_open()) walStream.close();
    }

    void set(const std::string& key, const std::string& value) override {
        std::lock_guard<std::mutex> lock(storageMutex);
        writeAheadLog(key, value);
        memTable[key] = value;

        if(memTable.size() >= MEMTABLE_THRESHOLD) {
            // flushToDisk expects the caller to hold the lock (no internal locking)
            flushToDisk();
        }
    }

    std::string get(const std::string& key) override {
        std::lock_guard<std::mutex> lock(storageMutex);
        // 1) check memTable first
        auto mit = memTable.find(key);
        if(mit != memTable.end()) {
            const std::string &v = mit->second;
            return (v == TOMBSTONE) ? std::string() : v;
        }

        // 2) if not in memTable, check index -> SSTable
        if(index.find(key) == index.end()) { 
            return std::string(); // not found
        }

        std::ifstream sstable(sstableFile);
        if(!sstable) {
            return std::string();
        }

        sstable.seekg(index[key]);
        std::string k, v;
        if(!(sstable >> k >> v)) {
            return std::string();
        }
        if(v == TOMBSTONE) return std::string();
        return v;
    }

    void del(const std::string& key) override {
        std::lock_guard<std::mutex> lock(storageMutex);
        writeAheadLog(key, TOMBSTONE);
        memTable[key] = TOMBSTONE;
        if(memTable.size() >= MEMTABLE_THRESHOLD) {
            flushToDisk();
        }
    }

    void recover() {
        std::ifstream wal(walFile);
        if(!wal) {
            return;
        }
        std::string key, value;
        // Read WAL sequentially; later entries override previous
        while(wal >> key >> value) {
            // keep deletions as tombstones so they will be flushed to SSTable
            memTable[key] = value;
        }
        wal.close();
    }

    void compact() {
        std::lock_guard<std::mutex> lock(storageMutex);
        std::ifstream sstable(sstableFile);
        if (!sstable.is_open()) {
            std::cerr << "No SSTable to compact.\n";
            return;
        }

        std::map<std::string, std::string> latestEntries;
        std::string key, value;

        // Read all existing key-value pairs
        while (sstable >> key >> value) {
            latestEntries[key] = value; // keep the latest occurrence
        }
        sstable.close();

        // Remove deleted entries (marked by tombstone)
        for (auto it = latestEntries.begin(); it != latestEntries.end();) {
            if (it->second == TOMBSTONE) {
                it = latestEntries.erase(it);
            } else {
                ++it;
            }
        }

        // Rewrite compacted SSTable
        std::ofstream newSstable(sstableFile, std::ios::trunc);
        if (!newSstable.is_open()) {
            std::cerr << "Error creating new SSTable.\n";
            return;
        }

        index.clear();
        for (const auto& pair : latestEntries) {
            index[pair.first] = newSstable.tellp();
            newSstable << pair.first << " " << pair.second << "\n";
        }

        newSstable.close();
        std::cout << "Compaction complete. SSTable size reduced to " 
                  << latestEntries.size() << " entries.\n";
        // index is already rebuilt above, but you could alternatively call buildIndex()
    }
};
