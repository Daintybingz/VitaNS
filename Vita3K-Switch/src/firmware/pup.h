#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <filesystem>
#include <vector>
#include <map>

namespace fs = std::filesystem;

namespace firmware {

// Structure to hold firmware information
struct FirmwareInfo {
    std::string version;
    uint32_t build_number;
    bool is_complete;
    std::vector<std::string> installed_components;
};

// Structure to hold PUP file header
struct PUPHeader {
    char magic[4];         // "SCEUF"
    uint32_t version;      // PUP version
    uint32_t firmware_version;
    uint32_t build_number;
    uint32_t file_count;
};

// Structure to hold PUP file entry
struct PUPFileEntry {
    uint64_t file_type;
    uint64_t offset;
    uint64_t size;
    uint64_t flags;
};

// Firmware component types
enum class FirmwareComponent {
    OS0,    // Core OS files
    VS0,    // System applications and modules
    SA0,    // Fonts and system assets
    PD0     // System data
};

// Function to install a PUP file
// Returns the firmware version string if successful, empty string otherwise
std::string install_pup(const fs::path &pref_path, const fs::path &pup_path, 
                       const std::function<void(uint32_t)> &progress_callback = nullptr);

// Function to check if firmware is installed
bool is_firmware_installed(const fs::path &pref_path);

// Function to get firmware information
FirmwareInfo get_firmware_info(const fs::path &pref_path);

// Function to extract a PUP file
bool extract_pup(const fs::path &pup_path, const fs::path &output_path,
                const std::function<void(uint32_t)> &progress_callback = nullptr);

// Function to decrypt PUP packages
bool decrypt_packages(const fs::path &input_path, const fs::path &output_path,
                     const std::function<void(uint32_t)> &progress_callback = nullptr);

// Function to extract firmware components
bool extract_components(const fs::path &input_path, const fs::path &output_path,
                       const std::function<void(uint32_t)> &progress_callback = nullptr);

// Function to mount firmware components
bool mount_components(const fs::path &pref_path);

// Function to unmount firmware components
bool unmount_components();

} // namespace firmware
