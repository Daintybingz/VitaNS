#include "vpk.h"
#include "../filesystem/vita_filesystem.h"

#include <cstdio>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

// We'll use miniz for ZIP handling (a lightweight ZIP library)
// In a real implementation, you'd include the miniz library or another ZIP library
// For this example, we'll stub out the ZIP functionality

// Simple file utilities to avoid filesystem compatibility issues
namespace FileUtils {
    // Check if file exists
    bool fileExists(const std::string& path) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return true;
        }
        return false;
    }
    
    // Get file extension
    std::string getExtension(const std::string& path) {
        size_t pos = path.find_last_of(".");
        if (pos != std::string::npos) {
            return path.substr(pos);
        }
        return "";
    }
    
    // Get filename from path
    std::string getFilename(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        }
        return path;
    }
    
    // Get directory path
    std::string getDirectory(const std::string& path) {
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos) {
            return path.substr(0, pos);
        }
        return "";
    }
    
    // Create directory
    bool createDirectory(const std::string& path) {
        return mkdir(path.c_str(), 0755) == 0;
    }
    
    // Create directory tree
    bool createDirectoryTree(const std::string& path) {
        std::string current;
        std::string remaining = path;
        
        // Handle absolute paths
        if (path.size() > 0 && (path[0] == '/' || path[0] == '\\')) {
            current = "/";
            remaining = path.substr(1);
        }
        
        size_t pos = 0;
        while ((pos = remaining.find_first_of("/\\")) != std::string::npos) {
            current += remaining.substr(0, pos);
            remaining = remaining.substr(pos + 1);
            
            if (!current.empty() && !fileExists(current)) {
                if (!createDirectory(current)) {
                    return false;
                }
            }
            
            current += "/";
        }
        
        if (!remaining.empty()) {
            current += remaining;
            if (!fileExists(current)) {
                return createDirectory(current);
            }
        }
        
        return true;
    }
}

VPKParser::VPKParser() {}

VPKParser::~VPKParser() {}

bool VPKParser::parse(const std::string& path) {
    vpkPath = path;
    
    // Check if file exists
    if (!FileUtils::fileExists(path)) {
        printf("[VPKParser] File not found: %s\n", path.c_str());
        return false;
    }
    
    // Check file extension
    std::string ext = FileUtils::getExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext != ".vpk") {
        printf("[VPKParser] Not a VPK file: %s\n", path.c_str());
        return false;
    }
    
    printf("[VPKParser] Parsing VPK file: %s\n", path.c_str());
    
    // TODO: Implement actual ZIP parsing using miniz or another ZIP library
    // For now, we'll stub this out with dummy data
    
    // Simulate parsing the VPK file
    entries.clear();
    
    // Add some dummy entries
    VPKEntry eboot;
    eboot.filename = "eboot.bin";
    eboot.offset = 0;
    eboot.size = 1024 * 1024; // 1MB
    eboot.compressedSize = 512 * 1024; // 512KB
    eboot.isCompressed = true;
    entries[eboot.filename] = eboot;
    
    VPKEntry paramSfo;
    paramSfo.filename = "sce_sys/param.sfo";
    paramSfo.offset = eboot.offset + eboot.compressedSize;
    paramSfo.size = 4 * 1024; // 4KB
    paramSfo.compressedSize = 2 * 1024; // 2KB
    paramSfo.isCompressed = true;
    entries[paramSfo.filename] = paramSfo;
    
    // Parse param.sfo for metadata
    if (!parseParamSFO()) {
        printf("[VPKParser] Warning: Failed to parse param.sfo\n");
        // Continue anyway, as we can still extract files
    }
    
    printf("[VPKParser] VPK parsed successfully. Found %zu files.\n", entries.size());
    return true;
}

bool VPKParser::extract(const std::string& outputPath, bool overwrite) {
    if (entries.empty()) {
        printf("[VPKParser] No VPK loaded or VPK is empty\n");
        return false;
    }
    
    // Create output directory if it doesn't exist
    if (!FileUtils::fileExists(outputPath)) {
        if (!FileUtils::createDirectoryTree(outputPath)) {
            printf("[VPKParser] Failed to create output directory: %s\n", outputPath.c_str());
            return false;
        }
    }
    
    printf("[VPKParser] Extracting VPK to: %s\n", outputPath.c_str());
    
    // Extract all files
    bool success = true;
    for (const auto& entry : entries) {
        std::string outPath = outputPath + "/" + entry.first;
        
        // Create directory if needed
        std::string dirPath = FileUtils::getDirectory(outPath);
        if (!dirPath.empty() && !FileUtils::fileExists(dirPath)) {
            if (!FileUtils::createDirectoryTree(dirPath)) {
                printf("[VPKParser] Failed to create directory: %s\n", dirPath.c_str());
                success = false;
                continue;
            }
        }
        
        // Check if file exists and overwrite flag
        if (FileUtils::fileExists(outPath) && !overwrite) {
            printf("[VPKParser] Skipping existing file: %s\n", entry.first.c_str());
            continue;
        }
        
        // Extract file
        if (!extractFile(entry.first, outPath)) {
            printf("[VPKParser] Failed to extract file: %s\n", entry.first.c_str());
            success = false;
        }
    }
    
    printf("[VPKParser] Extraction %s\n", success ? "completed successfully" : "completed with errors");
    return success;
}

bool VPKParser::extractFile(const std::string& filename, const std::string& outputPath) {
    // Check if file exists in VPK
    auto it = entries.find(filename);
    if (it == entries.end()) {
        printf("[VPKParser] File not found in VPK: %s\n", filename.c_str());
        return false;
    }
    
    // TODO: Implement actual file extraction using miniz or another ZIP library
    // For now, we'll create a dummy file
    
    // Create directory if needed
    std::string dirPath = FileUtils::getDirectory(outputPath);
    if (!dirPath.empty()) {
        if (!FileUtils::createDirectoryTree(dirPath)) {
            printf("[VPKParser] Failed to create directory: %s\n", dirPath.c_str());
            return false;
        }
    }
    
    // Create dummy file
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile.is_open()) {
        printf("[VPKParser] Failed to create output file: %s\n", outputPath.c_str());
        return false;
    }
    
    // Write dummy data
    std::vector<uint8_t> dummyData(100, 0);
    outFile.write(reinterpret_cast<const char*>(dummyData.data()), dummyData.size());
    
    printf("[VPKParser] Extracted file: %s\n", filename.c_str());
    return true;
}

const VPKMetadata& VPKParser::getMetadata() const {
    return metadata;
}

std::vector<std::string> VPKParser::getFileList() const {
    std::vector<std::string> fileList;
    fileList.reserve(entries.size());
    
    for (const auto& entry : entries) {
        fileList.push_back(entry.first);
    }
    
    return fileList;
}

bool VPKParser::hasFile(const std::string& filename) const {
    return entries.find(filename) != entries.end();
}

bool VPKParser::getFileData(const std::string& filename, std::vector<uint8_t>& data) const {
    // Check if file exists in VPK
    auto it = entries.find(filename);
    if (it == entries.end()) {
        return false;
    }
    
    // TODO: Implement actual file extraction using miniz or another ZIP library
    // For now, we'll return dummy data
    
    data.resize(100, 0);
    return true;
}

bool VPKParser::install(VitaFileSystem& fs, const std::string& appPath) {
    if (entries.empty() || metadata.titleId.empty()) {
        printf("[VPKParser] No valid VPK loaded\n");
        return false;
    }
    
    // Create app directory
    std::string appDir = appPath + "/" + metadata.titleId;
    if (!fs.createDirectory(appDir)) {
        printf("[VPKParser] Failed to create app directory: %s\n", appDir.c_str());
        return false;
    }
    
    printf("[VPKParser] Installing VPK to: %s\n", appDir.c_str());
    
    // Extract all files to app directory
    bool success = true;
    for (const auto& entry : entries) {
        std::string outPath = appDir + "/" + entry.first;
        
        // Get file data
        std::vector<uint8_t> data;
        if (!getFileData(entry.first, data)) {
            printf("[VPKParser] Failed to get file data: %s\n", entry.first.c_str());
            success = false;
            continue;
        }
        
        // Write file
        if (!fs.writeFile(outPath, data, true)) {
            printf("[VPKParser] Failed to write file: %s\n", outPath.c_str());
            success = false;
        }
    }
    
    printf("[VPKParser] Installation %s\n", success ? "completed successfully" : "completed with errors");
    return success;
}

bool VPKParser::parseSFO(const std::vector<uint8_t>& data) {
    // TODO: Implement actual SFO parsing
    // For now, we'll set dummy metadata
    
    metadata.title = "Sample Game";
    metadata.titleId = "ABCD12345";
    metadata.version = "1.00";
    metadata.category = "gd";
    metadata.minFirmware = "3.60";
    
    return true;
}

bool VPKParser::parseParamSFO() {
    // Check if param.sfo exists in VPK
    if (!hasFile("sce_sys/param.sfo")) {
        printf("[VPKParser] param.sfo not found in VPK\n");
        return false;
    }
    
    // Get param.sfo data
    std::vector<uint8_t> sfoData;
    if (!getFileData("sce_sys/param.sfo", sfoData)) {
        printf("[VPKParser] Failed to extract param.sfo\n");
        return false;
    }
    
    // Parse SFO data
    return parseSFO(sfoData);
}
