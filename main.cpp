#include <iostream>

#include "db.h"
#include "commands.h"

// Example usage
int main() {
    
    try {
        
        db::DatabaseManager db("./content_db");

        commands::CommandProcessor processor = commands::CommandProcessor(&db);
        processor.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}