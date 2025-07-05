#include "param_sfo.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

ParamSfoParser::ParamSfoParser() {
}

ParamSfoParser::~ParamSfoParser() {
}

bool ParamSfoParser::parse(const std::string& path) {
    // Open the file
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        printf("[ParamSfoParser] Failed to open file: %s\n", path.c_str());
        return false;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Read the entire file
    std::vector<uint8_t> data(fileSize);
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    
    // Parse the data
    return parseData(data);
}

bool ParamSfoParser::parseData(const std::vector<uint8_t>& data) {
    // Clear previous data
    stringValues.clear();
    intValues.clear();
    
    // Check if data is large enough to contain a header
    if (data.size() < sizeof(SfoHeader)) {
        printf("[ParamSfoParser] Data too small to contain SFO header\n");
        return false;
    }
    
    // Read header
    SfoHeader header;
    std::memcpy(&header, data.data(), sizeof(SfoHeader));
    
    // Check magic
    const uint32_t SFO_MAGIC = 0x46535000; // "\0PSF" in little endian
    if (header.magic != SFO_MAGIC) {
        printf("[ParamSfoParser] Invalid SFO magic: 0x%08X\n", header.magic);
        return false;
    }
    
    // Parse index table entries
    for (uint32_t i = 0; i < header.tables_entries; i++) {
        // Calculate entry offset
        size_t entryOffset = sizeof(SfoHeader) + i * sizeof(SfoIndexTableEntry);
        
        // Check if data is large enough to contain this entry
        if (data.size() < entryOffset + sizeof(SfoIndexTableEntry)) {
            printf("[ParamSfoParser] Data too small to contain index table entry %u\n", i);
            return false;
        }
        
        // Read entry
        SfoIndexTableEntry entry;
        std::memcpy(&entry, data.data() + entryOffset, sizeof(SfoIndexTableEntry));
        
        // Calculate key and data offsets
        size_t keyOffset = header.key_table_offset + entry.key_offset;
        size_t dataOffset = header.data_table_offset + entry.data_offset;
        
        // Check if data is large enough to contain key and data
        if (data.size() < keyOffset || data.size() < dataOffset + entry.data_max_len) {
            printf("[ParamSfoParser] Data too small to contain key or data for entry %u\n", i);
            continue;
        }
        
        // Read key
        std::string key = readString(data, keyOffset, 256);
        
        // Read data based on format
        switch (static_cast<SfoDataFormat>(entry.data_fmt)) {
            case SfoDataFormat::UTF8:
            case SfoDataFormat::UTF8_SPECIAL: {
                std::string value = readString(data, dataOffset, entry.data_max_len);
                stringValues[key] = value;
                break;
            }
            case SfoDataFormat::INT32: {
                int32_t value = readInt32(data, dataOffset);
                intValues[key] = value;
                break;
            }
            default:
                printf("[ParamSfoParser] Unknown data format: 0x%04X for key %s\n", entry.data_fmt, key.c_str());
                break;
        }
    }
    
    // Parse metadata from values
    parseMetadata();
    
    return true;
}

GameMetadata ParamSfoParser::getMetadata() const {
    return metadata;
}

std::string ParamSfoParser::getString(const std::string& key) const {
    auto it = stringValues.find(key);
    if (it != stringValues.end()) {
        return it->second;
    }
    return "";
}

int32_t ParamSfoParser::getInt(const std::string& key, int32_t defaultValue) const {
    auto it = intValues.find(key);
    if (it != intValues.end()) {
        return it->second;
    }
    return defaultValue;
}

bool ParamSfoParser::hasKey(const std::string& key) const {
    return stringValues.find(key) != stringValues.end() || intValues.find(key) != intValues.end();
}

std::vector<std::string> ParamSfoParser::getKeys() const {
    std::vector<std::string> keys;
    
    for (const auto& pair : stringValues) {
        keys.push_back(pair.first);
    }
    
    for (const auto& pair : intValues) {
        if (std::find(keys.begin(), keys.end(), pair.first) == keys.end()) {
            keys.push_back(pair.first);
        }
    }
    
    return keys;
}

void ParamSfoParser::parseMetadata() {
    // Extract metadata from parsed values
    metadata.title = getString("TITLE");
    metadata.titleId = getString("TITLE_ID");
    metadata.category = getString("CATEGORY");
    metadata.contentId = getString("CONTENT_ID");
    metadata.appVersion = getString("APP_VER");
    metadata.minFwVersion = getString("PSP2_SYSTEM_VER");
    metadata.appType = getInt("APP_TYPE");
    metadata.attribute = getInt("ATTRIBUTE");
    metadata.parental = getString("PARENTAL_LEVEL");
    metadata.saveDataTitle = getString("SAVEDATA_TITLE");
    
    // If title is empty, use STITLE (short title) if available
    if (metadata.title.empty()) {
        metadata.title = getString("STITLE");
    }
}

std::string ParamSfoParser::readString(const std::vector<uint8_t>& data, size_t offset, size_t maxLen) {
    std::string result;
    
    // Read until null terminator or max length
    for (size_t i = 0; i < maxLen; i++) {
        if (offset + i >= data.size()) {
            break;
        }
        
        char c = static_cast<char>(data[offset + i]);
        if (c == '\0') {
            break;
        }
        
        result.push_back(c);
    }
    
    return result;
}

int32_t ParamSfoParser::readInt32(const std::vector<uint8_t>& data, size_t offset) {
    int32_t result = 0;
    
    if (offset + sizeof(int32_t) <= data.size()) {
        std::memcpy(&result, data.data() + offset, sizeof(int32_t));
    }
    
    return result;
}
