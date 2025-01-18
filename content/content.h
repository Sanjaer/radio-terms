#ifndef CONTENT_H
#define CONTENT_H

#include <string>
#include <vector>

namespace content {
    // Structure to hold our content
struct Content {
    std::string description;
    std::vector<std::string> hyperlinks;
    std::vector<std::string> imagePaths;

    // Serialize the content into a binary format
    std::vector<char> serialize() const;
    
    // Deserialize from binary format
    static Content deserialize(const char* data, size_t size) ;

    std::string GetFormated() const ;

    void Print();

};

} // namespace content

#endif // CONTENT_H