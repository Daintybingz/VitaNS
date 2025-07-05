#include "pup_extract.h"
#include <spdlog/spdlog.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdint>

namespace firmware {

// Minimal PUP header structure (adapted from vita3k/packages/src/pup.cpp)
struct PupHeader {
    char magic[8];
    uint32_t version;
    uint32_t file_count;
    uint32_t header_size;
    uint32_t entry_size;
    uint32_t unknown[3];
};

struct PupEntry {
    char name[32];
    uint32_t offset;
    uint32_t size;
    uint32_t flags;
    uint32_t unknown[4];
};

bool extract_pup_firmware(const std::string& pup_path, const std::string& target_dir) {
    namespace fs = std::filesystem;
    std::ifstream pup(pup_path, std::ios::binary);
    if (!pup.is_open()) {
        spdlog::error("Failed to open PUP file: {}", pup_path);
        return false;
    }
    PupHeader header{};
    pup.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (strncmp(header.magic, "SCEUF", 4) != 0) {
        spdlog::error("Invalid PUP magic");
        return false;
    }

    // Seek to entries
    pup.seekg(header.header_size, std::ios::beg);
    std::vector<PupEntry> entries(header.file_count);
    for (uint32_t i = 0; i < header.file_count; ++i) {
        pup.read(reinterpret_cast<char*>(&entries[i]), sizeof(PupEntry));
    }

    // Extract each entry
    for (const auto& entry : entries) {
        std::string entry_name(entry.name, strnlen(entry.name, sizeof(entry.name)));
        fs::path out_path = fs::path(target_dir) / entry_name;
        spdlog::info("Extracting {} (offset {} size {}) to {}", entry_name, entry.offset, entry.size, out_path.string());
        pup.seekg(entry.offset, std::ios::beg);
        std::vector<char> buffer(entry.size);
        pup.read(buffer.data(), entry.size);
        fs::create_directories(out_path.parent_path());
        std::ofstream out(out_path, std::ios::binary);
        if (!out.is_open()) {
            spdlog::error("Failed to create output file: {}", out_path.string());
            continue;
        }
        out.write(buffer.data(), entry.size);
        out.close();
    }
    spdlog::info("PUP extraction complete.");
    return true;
}

} 