#include "content.h"

#include <sstream>
#include <iostream>
#include <cstring> // memcpy
#include <stdint.h>

namespace content {

// Serialize the content into a binary format
std::vector<char> Content::serialize() const {
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
Content Content::deserialize(const char* data, size_t size) {
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

std::string Content::GetFormated() const {
    std::stringstream ss;
    ss << "Description: " << description << std::endl;
    ss << "Links:" << std::endl;
    for (const auto& link : hyperlinks) {
        ss << "- " << link << std::endl;
    }
    return ss.str();
}

void Content::Print(){
    std::cout << GetFormated() << std::endl;
}

} // namespace content