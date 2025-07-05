#include "vita_filesystem.h"
#include <cstdio>
#include <fstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

// Singleton instance
VitaFileSystem& VitaFileSystem::getInstance() {
    static VitaFileSystem instance;
    return instance;
}

VitaFileSystem::VitaFileSystem() : initialized(false) {}

VitaFileSystem::~VitaFileSystem() {
    finalize();
}

bool VitaFileSystem::initialize(const std::string& path) {
    if (initialized) {
        return true;
    }

    basePath = path;
    
    // Create base directory if it doesn't exist
    if (!fs::exists(basePath)) {
        try {
            fs::create_directories(basePath);
        } catch (const std::exception& e) {
            printf("[VitaFileSystem] Failed to create base directory: %s\n", e.what());
            return false;
        }
    }
    
    // Mount default paths
    mountPath(VITA_UX0_PATH, basePath + "/ux0");
    mountPath(VITA_APP_PATH, basePath + "/ux0/app");
    mountPath(VITA_SAVEDATA_PATH, basePath + "/ux0/user/00/savedata");
    mountPath(VITA_ADDCONT_PATH, basePath + "/ux0/addcont");
    mountPath(VITA_TEMP_PATH, basePath + "/ux0/temp");
    
    // Create default directories
    createDirectory(VITA_APP_PATH);
    createDirectory(VITA_SAVEDATA_PATH);
    createDirectory(VITA_ADDCONT_PATH);
    createDirectory(VITA_TEMP_PATH);
    
    initialized = true;
    printf("[VitaFileSystem] Initialized with base path: %s\n", basePath.c_str());
    return true;
}

void VitaFileSystem::finalize() {
    if (!initialized) {
        return;
    }
    
    mountPoints.clear();
    initialized = false;
}

bool VitaFileSystem::mountPath(const std::string& vitaPath, const std::string& hostPath) {
    // Normalize paths
    std::string normalizedVitaPath = normalizePath(vitaPath);
    
    // Create host directory if it doesn't exist
    if (!fs::exists(hostPath)) {
        try {
            fs::create_directories(hostPath);
        } catch (const std::exception& e) {
            printf("[VitaFileSystem] Failed to create directory for mount point: %s\n", e.what());
            return false;
        }
    }
    
    // Add to mount points
    mountPoints[normalizedVitaPath] = hostPath;
    printf("[VitaFileSystem] Mounted %s to %s\n", normalizedVitaPath.c_str(), hostPath.c_str());
    return true;
}

bool VitaFileSystem::unmountPath(const std::string& vitaPath) {
    std::string normalizedPath = normalizePath(vitaPath);
    auto it = mountPoints.find(normalizedPath);
    
    if (it != mountPoints.end()) {
        mountPoints.erase(it);
        printf("[VitaFileSystem] Unmounted %s\n", normalizedPath.c_str());
        return true;
    }
    
    return false;
}

std::string VitaFileSystem::resolveVitaPath(const std::string& vitaPath) const {
    std::string normalizedPath = normalizePath(vitaPath);
    
    // Find the longest matching mount point
    std::string matchedVitaPath;
    std::string matchedHostPath;
    
    for (const auto& mount : mountPoints) {
        if (normalizedPath.find(mount.first) == 0 && mount.first.length() > matchedVitaPath.length()) {
            matchedVitaPath = mount.first;
            matchedHostPath = mount.second;
        }
    }
    
    if (matchedVitaPath.empty()) {
        // No mount point found
        return "";
    }
    
    // Replace the Vita path prefix with the host path
    std::string relativePath = normalizedPath.substr(matchedVitaPath.length());
    
    // Ensure the relative path starts with a separator if needed
    if (!relativePath.empty() && relativePath[0] != '/' && relativePath[0] != '\\') {
        relativePath = "/" + relativePath;
    }
    
    // Convert to host path
    std::string hostPath = matchedHostPath + relativePath;
    
    // Replace forward slashes with the platform's preferred separator
    std::replace(hostPath.begin(), hostPath.end(), '/', fs::path::preferred_separator);
    
    return hostPath;
}

bool VitaFileSystem::fileExists(const std::string& path) const {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    return fs::exists(hostPath) && fs::is_regular_file(hostPath);
}

size_t VitaFileSystem::getFileSize(const std::string& path) const {
    if (!initialized) {
        return 0;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return 0;
    }
    
    try {
        return fs::file_size(hostPath);
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to get file size: %s\n", e.what());
        return 0;
    }
}

bool VitaFileSystem::readFile(const std::string& path, std::vector<uint8_t>& data) const {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    try {
        // Open file
        std::ifstream file(hostPath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Get file size
        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Read data
        data.resize(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        
        return true;
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to read file: %s\n", e.what());
        return false;
    }
}

bool VitaFileSystem::writeFile(const std::string& path, const std::vector<uint8_t>& data, bool createDirs) {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    try {
        // Create directories if needed
        if (createDirs) {
            fs::path filePath(hostPath);
            fs::create_directories(filePath.parent_path());
        }
        
        // Open file
        std::ofstream file(hostPath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Write data
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        
        return true;
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to write file: %s\n", e.what());
        return false;
    }
}

bool VitaFileSystem::copyFile(const std::string& src, const std::string& dst) {
    if (!initialized) {
        return false;
    }
    
    std::string srcHostPath = resolveVitaPath(src);
    std::string dstHostPath = resolveVitaPath(dst);
    
    if (srcHostPath.empty() || dstHostPath.empty()) {
        return false;
    }
    
    try {
        // Create destination directory if needed
        fs::path dstPath(dstHostPath);
        fs::create_directories(dstPath.parent_path());
        
        // Copy file
        fs::copy_file(srcHostPath, dstHostPath, fs::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to copy file: %s\n", e.what());
        return false;
    }
}

bool VitaFileSystem::removeFile(const std::string& path) {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    try {
        return fs::remove(hostPath);
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to remove file: %s\n", e.what());
        return false;
    }
}

bool VitaFileSystem::createDirectory(const std::string& path) {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    try {
        return fs::create_directories(hostPath);
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to create directory: %s\n", e.what());
        return false;
    }
}

bool VitaFileSystem::directoryExists(const std::string& path) const {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    return fs::exists(hostPath) && fs::is_directory(hostPath);
}

bool VitaFileSystem::removeDirectory(const std::string& path, bool recursive) {
    if (!initialized) {
        return false;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    try {
        if (recursive) {
            return fs::remove_all(hostPath) > 0;
        } else {
            return fs::remove(hostPath);
        }
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to remove directory: %s\n", e.what());
        return false;
    }
}

std::vector<std::string> VitaFileSystem::listDirectory(const std::string& path) const {
    std::vector<std::string> result;
    
    if (!initialized) {
        return result;
    }
    
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty() || !fs::exists(hostPath) || !fs::is_directory(hostPath)) {
        return result;
    }
    
    try {
        for (const auto& entry : fs::directory_iterator(hostPath)) {
            result.push_back(entry.path().filename().string());
        }
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to list directory: %s\n", e.what());
    }
    
    return result;
}

bool VitaFileSystem::isValidGamePath(const std::string& path) const {
    // TODO: Implement proper validation of PS Vita game files
    // This is a stub implementation
    
    // Check if path exists
    if (!fs::exists(path)) {
        return false;
    }
    
    // For now, just check if it's a directory or a file with a valid extension
    if (fs::is_directory(path)) {
        // Check for EBOOT.BIN or other indicators of a valid game
        return fs::exists(path + "/eboot.bin") || fs::exists(path + "/EBOOT.BIN");
    } else {
        // Check file extension
        std::string ext = fs::path(path).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return ext == ".vpk" || ext == ".psvita" || ext == ".zip";
    }
}

std::string VitaFileSystem::getGameID(const std::string& path) const {
    // TODO: Implement proper game ID extraction from PS Vita game files
    // This is a stub implementation
    
    // For now, just return a dummy ID
    return "DUMMY00000";
}

std::string VitaFileSystem::getGameTitle(const std::string& path) const {
    // TODO: Implement proper game title extraction from PS Vita game files
    // This is a stub implementation
    
    // For now, just return the filename
    return fs::path(path).filename().string();
}

std::string VitaFileSystem::normalizePath(const std::string& path) const {
    std::string result = path;
    
    // Replace backslashes with forward slashes
    std::replace(result.begin(), result.end(), '\\', '/');
    
    // Remove trailing slash if present
    if (!result.empty() && result.back() == '/') {
        result.pop_back();
    }
    
    return result;
}

bool VitaFileSystem::createDirectoryTree(const std::string& path) {
    std::string hostPath = resolveVitaPath(path);
    if (hostPath.empty()) {
        return false;
    }
    
    try {
        return fs::create_directories(hostPath);
    } catch (const std::exception& e) {
        printf("[VitaFileSystem] Failed to create directory tree: %s\n", e.what());
        return false;
    }
}
