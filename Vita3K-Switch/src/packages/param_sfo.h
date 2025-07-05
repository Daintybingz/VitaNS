#pragma once

#include <string>
#include <map>
#include <vector>
#include <cstdint>

// PS Vita param.sfo format structures
struct SfoHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t key_table_offset;
    uint32_t data_table_offset;
    uint32_t tables_entries;
};

struct SfoIndexTableEntry {
    uint16_t key_offset;
    uint16_t data_fmt;
    uint32_t data_len;
    uint32_t data_max_len;
    uint32_t data_offset;
};

// Data format types
enum class SfoDataFormat {
    UTF8_SPECIAL = 0x0004,
    UTF8 = 0x0204,
    INT32 = 0x0404
};

// Game metadata structure
struct GameMetadata {
    std::string title;           // TITLE
    std::string titleId;         // TITLE_ID
    std::string category;        // CATEGORY
    std::string contentId;       // CONTENT_ID
    std::string appVersion;      // APP_VER
    std::string minFwVersion;    // PSP2_SYSTEM_VER
    uint32_t appType = 0;        // APP_TYPE
    uint32_t attribute = 0;      // ATTRIBUTE
    std::string parental;        // PARENTAL_LEVEL
    std::string saveDataTitle;   // SAVEDATA_TITLE
    
    // Additional metadata
    std::string iconPath;        // Path to icon file
    std::string bgPath;          // Path to background image
    std::string startupPath;     // Path to startup image
};

class ParamSfoParser {
public:
    ParamSfoParser();
    ~ParamSfoParser();
    
    // Parse param.sfo file
    bool parse(const std::string& path);
    bool parseData(const std::vector<uint8_t>& data);
    
    // Get metadata
    GameMetadata getMetadata() const;
    
    // Get specific values
    std::string getString(const std::string& key) const;
    int32_t getInt(const std::string& key, int32_t defaultValue = 0) const;
    
    // Check if key exists
    bool hasKey(const std::string& key) const;
    
    // Get all keys
    std::vector<std::string> getKeys() const;
    
private:
    // Internal data
    std::map<std::string, std::string> stringValues;
    std::map<std::string, int32_t> intValues;
    GameMetadata metadata;
    
    // Helper functions
    void parseMetadata();
    std::string readString(const std::vector<uint8_t>& data, size_t offset, size_t maxLen);
    int32_t readInt32(const std::vector<uint8_t>& data, size_t offset);
};
