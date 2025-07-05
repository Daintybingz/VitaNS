#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>

// PS Vita filesystem paths
constexpr const char* VITA_UX0_PATH = "ux0:";
constexpr const char* VITA_APP_PATH = "ux0:/app";
constexpr const char* VITA_SAVEDATA_PATH = "ux0:/user/00/savedata";
constexpr const char* VITA_ADDCONT_PATH = "ux0:/addcont";
constexpr const char* VITA_TEMP_PATH = "ux0:/temp";

// File access modes
enum class FileAccessMode {
    READ,
    WRITE,
    READWRITE,
    APPEND
};

// File system class for PS Vita emulation
class VitaFileSystem {
public:
    // Singleton pattern
    static VitaFileSystem& getInstance();

    // Initialize the filesystem
    bool initialize(const std::string& basePath);
    void finalize();

    // Mount points
    bool mountPath(const std::string& vitaPath, const std::string& hostPath);
    bool unmountPath(const std::string& vitaPath);
    std::string resolveVitaPath(const std::string& vitaPath) const;

    // File operations
    bool fileExists(const std::string& path) const;
    size_t getFileSize(const std::string& path) const;
    bool readFile(const std::string& path, std::vector<uint8_t>& data) const;
    bool writeFile(const std::string& path, const std::vector<uint8_t>& data, bool createDirs = true);
    bool copyFile(const std::string& src, const std::string& dst);
    bool removeFile(const std::string& path);

    // Directory operations
    bool createDirectory(const std::string& path);
    bool directoryExists(const std::string& path) const;
    bool removeDirectory(const std::string& path, bool recursive = false);
    std::vector<std::string> listDirectory(const std::string& path) const;

    // Game loading
    bool isValidGamePath(const std::string& path) const;
    std::string getGameID(const std::string& path) const;
    std::string getGameTitle(const std::string& path) const;

private:
    VitaFileSystem();
    ~VitaFileSystem();
    VitaFileSystem(const VitaFileSystem&) = delete;
    VitaFileSystem& operator=(const VitaFileSystem&) = delete;

    // Internal state
    bool initialized;
    std::string basePath;
    std::map<std::string, std::string> mountPoints;

    // Helper methods
    bool createDirectoryTree(const std::string& path);
    std::string normalizePath(const std::string& path) const;
};
