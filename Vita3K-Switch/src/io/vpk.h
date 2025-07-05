#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct VPKEntry {
    std::string filePath;
    uint32_t offset;
    uint32_t size;
};

class VPKExtractor {
public:
    VPKExtractor(const std::string& vpkPath);
    ~VPKExtractor();

    bool open();
    void close();
    bool isOpen() const;

    // List all files in the VPK archive
    std::vector<VPKEntry> listFiles();

    // Extract a single file to the target directory
    bool extractFile(const VPKEntry& entry, const std::string& targetDir);

    // Extract all files to the target directory
    bool extractAll(const std::string& targetDir);

private:
    std::string vpkPath;
    FILE* vpkFile;
    std::vector<VPKEntry> entries;
    bool parseVPK();
};
