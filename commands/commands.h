#ifndef COMMANDS_H
#define COMMANDS_H

#include "db.h"

namespace commands{

class CommandProcessor {
private:

    db::DatabaseManager *db;

    // Function to join remaining tokens into a single string
    std::string joinParameters(const std::string& input, size_t startPos);

    // Function to check if a string is a valid command
    bool isCommand(const std::string& str);

    std::string extractParameters(const std::string &line, const std::string &command);

    void addProcedure(const std::string& key);

    // Function to process the input line
    void processLine(const std::string& line);

public:

    CommandProcessor(db::DatabaseManager *db) : db(db){};

    void run();
};

} // namespace commands

#endif // COMMANDS_H