#include "pup.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace firmware {

// Constants
constexpr int SCEUF_HEADER_SIZE = 0x80;
constexpr int SCEUF_FILEREC_SIZE = 0x20;
constexpr int HEADER_LENGTH = 0x1000;

// File type mappings for PUP files
static const std::map<int, std::string> PUP_TYPES = {
    { 0x100, "version.txt" },
    { 0x101, "license.xml" },
    { 0x200, "psp2swu.self" },
    { 0x204, "cui_setupper.self" },
    { 0x400, "package_scewm.wm" },
    { 0x401, "package_sceas.as" },
    { 0x2005, "UpdaterES1.CpUp" },
    { 0x2006, "UpdaterES2.CpUp" },
};

// Filesystem type mappings
static const char *FSTYPE[] = {
    "unknown0",
    "os0",
    "unknown2",
    "unknown3",
    "vs0_chmod",
    "unknown5",
    "unknown6",
    "unknown7",
    "pervasive8",
    "boot_slb2",
    "vs0",
    "devkit_cp",
    "motionC",
    "bbmc",
    "unknownE",
    "motionF",
    "touch10",
    "touch11",
    "syscon12",
    "syscon13",
    "pervasive14",
    "unknown15",
    "vs0_tarpatch",
    "sa0",
    "pd0",
    "pervasive19",
    "unknown1A",
    "psp_emulist",
};

// Helper function to create a filename based on the header and file type
static std::string make_filename(unsigned char *hdr, int64_t filetype) {
    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t flags = 0;
    uint32_t moffs = 0;
    uint64_t metaoffs = 0;
    memcpy(&magic, &hdr[0], 4);
    memcpy(&version, &hdr[4], 4);
    memcpy(&flags, &hdr[8], 4);
    memcpy(&moffs, &hdr[12], 4);
    memcpy(&metaoffs, &hdr[16], 8);

    // Check for SCE magic and appropriate version/flags
    if (magic == 0x53434500 && version == 3 && flags == 0x30040) { // SCE magic is "SCE\0"
        std::vector<unsigned char> meta(hdr + metaoffs, hdr + (HEADER_LENGTH - metaoffs));
        unsigned char t = 0;
        memcpy(&t, &meta[4], 1);

        static int typecount = 0;

        if (t < 0x1C) { // 0x1C is the file separator
            std::string name = std::string(FSTYPE[t]) + "-" + 
                               (typecount < 10 ? "0" : "") + std::to_string(typecount) + ".pkg";
            typecount++;
            return name;
        }
    }
    
    // Default name for unknown file types
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "unknown-0x%llX.pkg", (unsigned long long)filetype);
    return std::string(buffer);
}

// Extract files from a PUP package
bool extract_pup(const fs::path &pup_path, const fs::path &output_path,
                const std::function<void(uint32_t)> &progress_callback) {
    // Create output directory if it doesn't exist
    if (!fs::exists(output_path)) {
        fs::create_directories(output_path);
    }

    // Open the PUP file
    std::ifstream infile(pup_path, std::ios::binary);
    if (!infile.is_open()) {
        std::cerr << "Failed to open PUP file: " << pup_path << std::endl;
        return false;
    }

    // Read the header
    char header[SCEUF_HEADER_SIZE];
    infile.read(header, SCEUF_HEADER_SIZE);

    // Verify the magic
    if (strncmp(header, "SCEUF", 5) != 0) {
        std::cerr << "Invalid PUP file: Magic mismatch" << std::endl;
        return false;
    }

    // Parse header information
    uint32_t cnt = 0;
    uint32_t pup_version = 0;
    uint32_t firmware_version = 0;
    uint32_t build_number = 0;
    memcpy(&cnt, &header[0x18], 4);
    memcpy(&pup_version, &header[8], 4);
    memcpy(&firmware_version, &header[0x10], 4);
    memcpy(&build_number, &header[0x14], 4);

    std::cout << "PUP Version: 0x" << std::hex << pup_version << std::endl;
    std::cout << "Firmware Version: 0x" << std::hex << firmware_version << std::endl;
    std::cout << "Build Number: " << std::dec << build_number << std::endl;
    std::cout << "Number Of Files: " << cnt << std::endl;

    // Extract each file
    for (uint32_t x = 0; x < cnt; x++) {
        // Update progress
        if (progress_callback) {
            progress_callback((x * 100) / cnt);
        }

        // Read file record
        infile.seekg(SCEUF_HEADER_SIZE + x * SCEUF_FILEREC_SIZE);
        char rec[SCEUF_FILEREC_SIZE];
        infile.read(rec, SCEUF_FILEREC_SIZE);

        // Parse file information
        uint64_t filetype = 0;
        uint64_t offset = 0;
        uint64_t length = 0;
        uint64_t flags = 0;

        memcpy(&filetype, &rec[0], 8);
        memcpy(&offset, &rec[8], 8);
        memcpy(&length, &rec[16], 8);
        memcpy(&flags, &rec[24], 8);

        // Determine output filename
        std::string outname;
        auto it = PUP_TYPES.find(static_cast<int>(filetype));
        if (it != PUP_TYPES.end()) {
            outname = it->second;
        } else {
            // Read file header for package files
            infile.seekg(offset);
            unsigned char hdr[HEADER_LENGTH];
            infile.read(reinterpret_cast<char*>(hdr), HEADER_LENGTH);
            outname = make_filename(hdr, filetype);
        }

        // Extract the file
        std::cout << "Extracting " << outname << " (type: 0x" << std::hex << filetype 
                  << ", size: " << std::dec << length << " bytes)" << std::endl;

        infile.seekg(offset);
        std::vector<char> buffer(length);
        infile.read(buffer.data(), length);

        fs::path outpath = output_path / outname;
        std::ofstream outfile(outpath, std::ios::binary);
        if (!outfile.is_open()) {
            std::cerr << "Failed to create output file: " << outpath << std::endl;
            return false;
        }

        outfile.write(buffer.data(), length);
        outfile.close();
    }

    // Update progress to 100%
    if (progress_callback) {
        progress_callback(100);
    }

    return true;
}

// Stub implementations for the remaining functions
// These will be implemented in future updates

bool decrypt_packages(const fs::path &input_path, const fs::path &output_path,
                     const std::function<void(uint32_t)> &progress_callback) {
    // This will be implemented in a future update
    // For now, just return false to indicate not implemented
    std::cerr << "decrypt_packages not yet implemented" << std::endl;
    return false;
}

bool extract_components(const fs::path &input_path, const fs::path &output_path,
                       const std::function<void(uint32_t)> &progress_callback) {
    // This will be implemented in a future update
    // For now, just return false to indicate not implemented
    std::cerr << "extract_components not yet implemented" << std::endl;
    return false;
}

std::string install_pup(const fs::path &pref_path, const fs::path &pup_path, 
                       const std::function<void(uint32_t)> &progress_callback) {
    // Create temporary directory for PUP extraction
    fs::path pup_dec_root = pref_path / "PUP_DEC";
    if (fs::exists(pup_dec_root)) {
        std::cout << "Path already exists, deleting it and reinstalling" << std::endl;
        fs::remove_all(pup_dec_root);
    }

    fs::create_directories(pup_dec_root);
    const auto pup_dest = pup_dec_root / "PUP";
    fs::create_directories(pup_dest);

    // Extract PUP files
    if (!extract_pup(pup_path, pup_dest, [&](uint32_t p) {
        if (progress_callback) progress_callback(p * 0.2); // 0-20%
    })) {
        std::cerr << "Failed to extract PUP files" << std::endl;
        return "";
    }

    // Create directory for decrypted files
    const auto pup_dec = pup_dec_root / "PUP_dec";
    fs::create_directories(pup_dec);

    // Decrypt packages (to be implemented)
    if (!decrypt_packages(pup_dest, pup_dec, [&](uint32_t p) {
        if (progress_callback) progress_callback(20 + p * 0.3); // 20-50%
    })) {
        std::cerr << "Failed to decrypt packages" << std::endl;
        return "";
    }

    // Extract components (to be implemented)
    if (!extract_components(pup_dec, pref_path, [&](uint32_t p) {
        if (progress_callback) progress_callback(50 + p * 0.5); // 50-100%
    })) {
        std::cerr << "Failed to extract components" << std::endl;
        return "";
    }

    // Get firmware version
    std::string fw_version;
    std::ifstream versionFile(pup_dest / "version.txt");
    if (versionFile.is_open()) {
        std::getline(versionFile, fw_version);
        versionFile.close();
    } else {
        std::cerr << "Firmware Version file not found!" << std::endl;
    }

    // Clean up temporary files
    fs::remove_all(pup_dec_root);

    return fw_version;
}

bool is_firmware_installed(const fs::path &pref_path) {
    // Check if essential firmware components exist
    return fs::exists(pref_path / "os0") &&
           fs::exists(pref_path / "vs0") &&
           fs::exists(pref_path / "sa0");
}

FirmwareInfo get_firmware_info(const fs::path &pref_path) {
    FirmwareInfo info;
    
    // Check if firmware is installed
    info.is_complete = is_firmware_installed(pref_path);
    
    // Get version from version.txt if it exists
    fs::path version_path = pref_path / "os0" / "version.txt";
    if (fs::exists(version_path)) {
        std::ifstream version_file(version_path);
        if (version_file.is_open()) {
            std::getline(version_file, info.version);
            version_file.close();
        }
    }
    
    // Check which components are installed
    if (fs::exists(pref_path / "os0")) info.installed_components.push_back("os0");
    if (fs::exists(pref_path / "vs0")) info.installed_components.push_back("vs0");
    if (fs::exists(pref_path / "sa0")) info.installed_components.push_back("sa0");
    if (fs::exists(pref_path / "pd0")) info.installed_components.push_back("pd0");
    
    return info;
}

bool mount_components(const fs::path &pref_path) {
    // This will be implemented in a future update
    // For now, just return false to indicate not implemented
    std::cerr << "mount_components not yet implemented" << std::endl;
    return false;
}

bool unmount_components() {
    // This will be implemented in a future update
    // For now, just return false to indicate not implemented
    std::cerr << "unmount_components not yet implemented" << std::endl;
    return false;
}

} // namespace firmware
