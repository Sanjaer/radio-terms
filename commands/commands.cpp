#include "commands.h"

#include <iostream>
#include <algorithm>
#include <sstream>

#include "content.h"

namespace commands{

// Function to join remaining tokens into a single string
std::string CommandProcessor::joinParameters(const std::string& input, size_t startPos) {
    if (startPos >= input.length()) return "";
    return input.substr(startPos);
}

// Function to check if a string is a valid command
bool CommandProcessor::isCommand(const std::string& str) {
    return str == "add" || str == "mod" || str == "rm";
}

std::string CommandProcessor::extractParameters(const std::string &line, const std::string &command){
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

void CommandProcessor::addProcedure(const std::string& key){
    content::Content content;
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
void CommandProcessor::processLine(const std::string& line) {
    if (line.empty()) return;

    std::string command;
    std::istringstream iss(line);
    iss >> command;

    content::Content content;

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

void CommandProcessor::run() {
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

} // namespace commands