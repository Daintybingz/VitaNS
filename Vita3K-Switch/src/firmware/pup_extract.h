#pragma once
#include <string>

namespace firmware {
// Extracts a PS Vita PUP file to the given target directory (e.g., /switch/vitans/firmware)
// Returns true on success, false on failure. Logs errors.
bool extract_pup_firmware(const std::string& pup_path, const std::string& target_dir);
} 