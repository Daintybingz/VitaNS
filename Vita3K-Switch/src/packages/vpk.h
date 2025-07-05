#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>

// Forward declarations
class VitaFileSystem;

// VPK file entry structure
struct VPKEntry {
    std::string filename;
    uint64_t offset;
    uint64_t size;
    uint64_t compressedSize;
    bool isCompressed;
};

// VPK metadata
struct VPKMetadata {
    std::string title;
    std::string titleId;
    std::string version;
    std::string category;
    std::string minFirmware;
};

// VPK parser class
class VPKParser {
public:
    // Constructor
    VPKParser();
    ~VPKParser();

    // Parse VPK file
    bool parse(const std::string& path);
    
    // Extract VPK file
    bool extract(const std::string& outputPath, bool overwrite = false);
    
    // Extract specific file from VPK
    bool extractFile(const std::string& filename, const std::string& outputPath);
    
    // Get metadata
    const VPKMetadata& getMetadata() const;
    
    // Get file list
    std::vector<std::string> getFileList() const;
    
    // Check if VPK contains a file
    bool hasFile(const std::string& filename) const;
    
    // Get file data
    bool getFileData(const std::string& filename, std::vector<uint8_t>& data) const;
    
    // Install VPK to the file system
    bool install(VitaFileSystem& fs, const std::string& appPath);

private:
    // VPK file path
    std::string vpkPath;
    
    // Metadata
    VPKMetadata metadata;
    
    // File entries
    std::map<std::string, VPKEntry> entries;
    
    // Parse SFO file for metadata
    bool parseSFO(const std::vector<uint8_t>& data);
    
    // Parse param.sfo for metadata
    bool parseParamSFO();
};
