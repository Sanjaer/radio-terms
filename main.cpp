#include <lmdb.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <algorithm>

// Structure to hold our content
struct Content {
    std::string description;
    std::vector<std::string> hyperlinks;
    std::vector<std::string> imagePaths;

    // Serialize the content into a binary format
    std::vector<char> serialize() const {
        std::vector<char> data;
        
        // Add description length and description
        uint32_t descLen = description.length();
        data.insert(data.end(), reinterpret_cast<char*>(&descLen), 
                   reinterpret_cast<char*>(&descLen) + sizeof(descLen));
        data.insert(data.end(), description.begin(), description.end());
        
        // Add hyperlinks
        uint32_t linkCount = hyperlinks.size();
        data.insert(data.end(), reinterpret_cast<char*>(&linkCount),
                   reinterpret_cast<char*>(&linkCount) + sizeof(linkCount));
        for (const auto& link : hyperlinks) {
            uint32_t linkLen = link.length();
            data.insert(data.end(), reinterpret_cast<char*>(&linkLen),
                       reinterpret_cast<char*>(&linkLen) + sizeof(linkLen));
            data.insert(data.end(), link.begin(), link.end());
        }
        
        // Add image paths
        uint32_t imageCount = imagePaths.size();
        data.insert(data.end(), reinterpret_cast<char*>(&imageCount),
                   reinterpret_cast<char*>(&imageCount) + sizeof(imageCount));
        for (const auto& path : imagePaths) {
            uint32_t pathLen = path.length();
            data.insert(data.end(), reinterpret_cast<char*>(&pathLen),
                       reinterpret_cast<char*>(&pathLen) + sizeof(pathLen));
            data.insert(data.end(), path.begin(), path.end());
        }
        
        return data;
    }

    // Deserialize from binary format
    static Content deserialize(const char* data, size_t size) {
        Content content;
        size_t offset = 0;
        
        // Read description
        uint32_t descLen;
        std::memcpy(&descLen, data + offset, sizeof(descLen));
        offset += sizeof(descLen);
        content.description = std::string(data + offset, descLen);
        offset += descLen;
        
        // Read hyperlinks
        uint32_t linkCount;
        std::memcpy(&linkCount, data + offset, sizeof(linkCount));
        offset += sizeof(linkCount);
        for (uint32_t i = 0; i < linkCount; i++) {
            uint32_t linkLen;
            std::memcpy(&linkLen, data + offset, sizeof(linkLen));
            offset += sizeof(linkLen);
            content.hyperlinks.push_back(std::string(data + offset, linkLen));
            offset += linkLen;
        }
        
        // Read image paths
        uint32_t imageCount;
        std::memcpy(&imageCount, data + offset, sizeof(imageCount));
        offset += sizeof(imageCount);
        for (uint32_t i = 0; i < imageCount; i++) {
            uint32_t pathLen;
            std::memcpy(&pathLen, data + offset, sizeof(pathLen));
            offset += sizeof(pathLen);
            content.imagePaths.push_back(std::string(data + offset, pathLen));
            offset += pathLen;
        }
        
        return content;
    }

    std::string GetFormated() const {
        std::stringstream ss;
        ss << "Description: " << description << std::endl;
        ss << "Links:" << std::endl;
        for (const auto& link : hyperlinks) {
            ss << "- " << link << std::endl;
        }
        return ss.str();
    }

    void Print(){
        std::cout << GetFormated() << std::endl;
    }

};

class DatabaseManager {
private:
    MDB_env* env;
    MDB_dbi dbi;
    std::string dbPath;

public:
    DatabaseManager(const std::string& path) : dbPath(path) {
        // Create directory if it doesn't exist
        std::filesystem::create_directories(path);
        
        // Initialize LMDB environment
        int rc = mdb_env_create(&env);
        if (rc != 0) {
            throw std::runtime_error("Failed to create LMDB environment");
        }
        
        // Set maximum database size (1GB should be more than enough for your use case)
        mdb_env_set_mapsize(env, 1UL * 1024UL * 1024UL * 1024UL);
        
        // Open environment
        rc = mdb_env_open(env, path.c_str(), 0, 0664);
        if (rc != 0) {
            mdb_env_close(env);
            throw std::runtime_error("Failed to open LMDB environment");
        }
        
        // Open database
        MDB_txn* txn;
        rc = mdb_txn_begin(env, nullptr, 0, &txn);
        if (rc != 0) {
            mdb_env_close(env);
            throw std::runtime_error("Failed to begin transaction");
        }
        
        rc = mdb_dbi_open(txn, nullptr, 0, &dbi);
        if (rc != 0) {
            mdb_txn_abort(txn);
            mdb_env_close(env);
            throw std::runtime_error("Failed to open database");
        }
        
        mdb_txn_commit(txn);
    }

    bool put(const std::string& key, const Content& content) {
        MDB_txn* txn;
        int rc = mdb_txn_begin(env, nullptr, 0, &txn);
        if (rc != 0) return false;
        
        // Serialize content
        std::vector<char> data = content.serialize();
        
        // Prepare key-value pairs
        MDB_val k, v;
        k.mv_size = key.length();
        k.mv_data = (void*)key.c_str();
        v.mv_size = data.size();
        v.mv_data = data.data();
        
        // Put data
        rc = mdb_put(txn, dbi, &k, &v, 0);
        if (rc != 0) {
            mdb_txn_abort(txn);
            return false;
        }
        
        return mdb_txn_commit(txn) == 0;
    }

    bool get(const std::string& key, Content& content) {
        MDB_txn* txn;
        int rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn);
        if (rc != 0) return false;
        
        MDB_val k, v;
        k.mv_size = key.length();
        k.mv_data = (void*)key.c_str();
        
        rc = mdb_get(txn, dbi, &k, &v);
        if (rc != 0) {
            mdb_txn_abort(txn);
            return false;
        }
        
        // Deserialize content
        content = Content::deserialize((char*)v.mv_data, v.mv_size);
        
        mdb_txn_abort(txn);
        return true;
    }

    bool remove(const std::string& key) {
        MDB_txn* txn;
        int rc = mdb_txn_begin(env, nullptr, 0, &txn);
        if (rc != 0) {
            std::cerr << "Failed to begin transaction: " << mdb_strerror(rc) << std::endl;
            return false;
        }
        
        // Prepare key
        MDB_val k;
        k.mv_size = key.length();
        k.mv_data = (void*)key.c_str();
        
        // Delete the entry
        rc = mdb_del(txn, dbi, &k, nullptr);
        if (rc != 0 && rc != MDB_NOTFOUND) {
            std::cerr << "Failed to delete entry: " << mdb_strerror(rc) << std::endl;
            mdb_txn_abort(txn);
            return false;
        }
        
        // Commit transaction
        rc = mdb_txn_commit(txn);
        if (rc != 0) {
            std::cerr << "Failed to commit transaction: " << mdb_strerror(rc) << std::endl;
            return false;
        }
        
        return true;
    }


    ~DatabaseManager() {
        mdb_dbi_close(env, dbi);
        mdb_env_close(env);
    }
};

class CommandProcessor {
private:

    DatabaseManager *db;

    // Function to join remaining tokens into a single string
    std::string joinParameters(const std::string& input, size_t startPos) {
        if (startPos >= input.length()) return "";
        return input.substr(startPos);
    }

    // Function to check if a string is a valid command
    bool isCommand(const std::string& str) {
        return str == "add" || str == "mod" || str == "rm";
    }

    std::string extractParameters(const std::string &line, const std::string &command){
        // Get the position after the command in the original string
        size_t commandPos = line.find(command);
        size_t paramStart = line.find_first_not_of(" \t", commandPos + command.length());
        
        std::string parameters = (paramStart != std::string::npos) ? 
                                joinParameters(line, paramStart) : "";

        // Process command with parameters
        std::cout << "Command: " << command << std::endl;
        std::cout << "Parameters: \"" << parameters << "\"" << std::endl;
        return parameters;
    }

    void addProcedure(const std::string& key){
        Content content;
        std::string line;
        std::cout << "Insert description: \n" << "> ";
        std::getline(std::cin, line);

        content.description = line;

        do {
            std::cout << "Insert hyperlink (empty intro to exit): \n" << "> ";
            std::getline(std::cin, line);
            content.hyperlinks.push_back(line);
        } while(!line.empty());
        
        if (db->put(key, content)){
            std::cout << "New entry created: " << key << std::endl;
            return;
        }
        std::cout << "error writing element to database: " << key << std::endl;
    }

    // Function to process the input line
    void processLine(const std::string& line) {
        if (line.empty()) return;

        std::string command;
        std::istringstream iss(line);
        iss >> command;

        Content content;

        // If no valid command, look for the entire string as an entry
        if (!isCommand(command)){
            std::cout << "No command. Full string: \"" << line << "\"" << std::endl;
            if (db->get(line, content)){
                content.Print();
                return;
            }
            std::cout << "error: entry not found" << std::endl;
            return;
        }

        // If it's a command, separate the entry from the key word and check if already present
        std::string parameter = extractParameters(line, command);
        if (parameter.empty()){
            std::cout << "Error: No term provided" << std::endl;
            return;
        }
        bool found = db->get(parameter, content);

        if (command == "add") {
            if (found) {
                std::cout << "error: entry already present" << std::endl;
                return;
            }
            // Add process
            addProcedure(parameter);
            return;
        }

        if (found && command == "mod") {
            // Modify Process
            return;
        }

        if (found && command == "rm") {
            // Remove proccess
            if(!db->remove(parameter)){
                std::cout << "error deleting entry" << std::endl;
            }
            return;
        }

        std::cout << "error: entry not found" << std::endl;
        return;
        
    }

public:

    CommandProcessor(DatabaseManager *db) : db(db){};

    void run() {
        std::string line;
        
        std::cout << "Enter commands (add/mod/rm) followed by parameters," << std::endl;
        std::cout << "or enter text without commands." << std::endl;
        std::cout << "Press Ctrl+C to exit." << std::endl;

        while (true) {
            std::cout << "> ";
            std::getline(std::cin, line);
            std::transform(line.begin(), line.end(), line.begin(),[](unsigned char c){ return std::tolower(c); });
            processLine(line);
        }
    }
};

// Example usage
int main() {
    
    try {
        
        DatabaseManager db("./content_db");

        CommandProcessor processor = CommandProcessor(&db);
        processor.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}