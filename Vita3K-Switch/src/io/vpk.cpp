#include "vpk.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <vector>

VPKExtractor::VPKExtractor(const std::string& vpkPath)
    : vpkPath(vpkPath), vpkFile(nullptr) {}

VPKExtractor::~VPKExtractor() {
    close();
}

bool VPKExtractor::open() {
    vpkFile = fopen(vpkPath.c_str(), "rb");
    if (!vpkFile) return false;
    return parseVPK();
}

void VPKExtractor::close() {
    if (vpkFile) {
        fclose(vpkFile);
        vpkFile = nullptr;
    }
    entries.clear();
}

bool VPKExtractor::isOpen() const {
    return vpkFile != nullptr;
}

std::vector<VPKEntry> VPKExtractor::listFiles() {
    return entries;
}

bool VPKExtractor::extractFile(const VPKEntry& entry, const std::string& targetDir) {
    if (!vpkFile) return false;
    std::filesystem::path outPath = std::filesystem::path(targetDir) / entry.filePath;
    std::filesystem::create_directories(outPath.parent_path());
    FILE* outFile = fopen(outPath.string().c_str(), "wb");
    if (!outFile) return false;
    fseek(vpkFile, entry.offset, SEEK_SET);
    std::vector<char> buffer(entry.size);
    fread(buffer.data(), 1, entry.size, vpkFile);
    fwrite(buffer.data(), 1, entry.size, outFile);
    fclose(outFile);
    return true;
}

bool VPKExtractor::extractAll(const std::string& targetDir) {
    if (!vpkFile) return false;
    for (const auto& entry : entries) {
        if (!extractFile(entry, targetDir)) return false;
    }
    return true;
}

// Minimal VPK parsing for demonstration (not full VPK spec)
bool VPKExtractor::parseVPK() {
    // This is a stub parser. Real VPKs need full directory & file chunk parsing.
    // For now, just scan for a simple header and fake one file entry for demo.
    fseek(vpkFile, 0, SEEK_END);
    long fileSize = ftell(vpkFile);
    fseek(vpkFile, 0, SEEK_SET);
    if (fileSize < 4) return false;
    char magic[5] = {0};
    fread(magic, 1, 4, vpkFile);
    if (strncmp(magic, "VPK\0", 4) != 0) return false;
    // Demo: pretend the rest is a single file
    VPKEntry entry;
    entry.filePath = "dummy.bin";
    entry.offset = 4;
    entry.size = fileSize - 4;
    entries.push_back(entry);
    return true;
}
