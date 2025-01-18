#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <lmdb.h>
#include <string>
#include <vector>

#include "content.h"

namespace db {

class DatabaseManager {
private:
    MDB_env* env;
    MDB_dbi dbi;
    std::string dbPath;

public:
    DatabaseManager(const std::string& path);

    bool put(const std::string& key, const content::Content& content);

    bool get(const std::string& key, content::Content& content);

    bool remove(const std::string& key);


    ~DatabaseManager();
};

} // namespace db

#endif // DB_MANAGER_H