#include "db.h"

#include <filesystem>
#include <iostream>

namespace db {

DatabaseManager::DatabaseManager(const std::string& path) : dbPath(path) {
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

bool DatabaseManager::put(const std::string& key, const content::Content& content) {
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

bool DatabaseManager::get(const std::string& key, content::Content& content) {
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
    content = content::Content::deserialize((char*)v.mv_data, v.mv_size);
    
    mdb_txn_abort(txn);
    return true;
}

bool DatabaseManager::remove(const std::string& key) {
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


DatabaseManager::~DatabaseManager() {
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);
}

} // namespace db