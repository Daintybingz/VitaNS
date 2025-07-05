#include "sceTrophy.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <filesystem>
#include <json.hpp>

namespace firmware {
namespace modules {
namespace trophy {

using json = nlohmann::json;
namespace fs = std::filesystem;

// Global trophy manager instance
static SceTrophyManager g_trophy_manager;

// Constructor
SceTrophyManager::SceTrophyManager()
    : initialized(false), next_context_id(1), next_handle_id(1) {
}

// Destructor
SceTrophyManager::~SceTrophyManager() {
    terminate();
}

// Initialize the trophy system
int SceTrophyManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        return SCE_TROPHY_ERROR_ALREADY_INITIALIZED;
    }
    
    printf("[SceTrophy] Initializing trophy system\n");
    
    // Create trophy directory if it doesn't exist
    fs::path trophy_path = fs::path("/switch/vitans/trophies");
    if (!fs::exists(trophy_path)) {
        try {
            fs::create_directories(trophy_path);
        } catch (const std::exception& e) {
            printf("[SceTrophy] Failed to create trophy directory: %s\n", e.what());
            return SCE_TROPHY_ERROR_IO;
        }
    }
    
    initialized = true;
    printf("[SceTrophy] Trophy system initialized successfully\n");
    
    return SCE_TROPHY_ERROR_NONE;
}

// Terminate the trophy system
int SceTrophyManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    printf("[SceTrophy] Terminating trophy system\n");
    
    // Save all trophy data
    for (const auto& handle_pair : handles) {
        save_trophy_data(handle_pair.first);
    }
    
    // Clear all data
    contexts.clear();
    handles.clear();
    trophy_data.clear();
    
    initialized = false;
    printf("[SceTrophy] Trophy system terminated\n");
    
    return SCE_TROPHY_ERROR_NONE;
}

// Create a trophy context
int SceTrophyManager::create_context(int *context_id, const char *comm_id, const char *comm_sign, int flags) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!context_id || !comm_id || !comm_sign) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    // Create a new context
    SceTrophyContext context;
    context.id = next_context_id++;
    context.comm_id = comm_id;
    context.comm_sign = comm_sign;
    context.flags = flags;
    context.initialized = true;
    
    contexts[context.id] = context;
    *context_id = context.id;
    
    printf("[SceTrophy] Created context %d for %s\n", context.id, comm_id);
    
    return SCE_TROPHY_ERROR_NONE;
}

// Destroy a trophy context
int SceTrophyManager::destroy_context(int context_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_context(context_id)) {
        return SCE_TROPHY_ERROR_INVALID_CONTEXT;
    }
    
    // Remove all handles associated with this context
    for (auto it = handles.begin(); it != handles.end();) {
        if (it->second.context_id == context_id) {
            int handle_id = it->first;
            it = handles.erase(it);
            
            // Remove trophy data for this handle
            trophy_data.erase(handle_id);
        } else {
            ++it;
        }
    }
    
    // Remove the context
    contexts.erase(context_id);
    
    printf("[SceTrophy] Destroyed context %d\n", context_id);
    
    return SCE_TROPHY_ERROR_NONE;
}

// Create a trophy handle
int SceTrophyManager::create_handle(int *handle_id, int context_id, const char *dir_name, int flags) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!handle_id || !dir_name) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    if (!is_valid_context(context_id)) {
        return SCE_TROPHY_ERROR_INVALID_CONTEXT;
    }
    
    // Create a new handle
    SceTrophyHandle handle;
    handle.id = next_handle_id++;
    handle.context_id = context_id;
    handle.path = std::string("/switch/vitans/trophies/") + dir_name;
    handle.initialized = true;
    
    handles[handle.id] = handle;
    *handle_id = handle.id;
    
    // Create trophy directory if it doesn't exist
    fs::path trophy_path = fs::path(handle.path);
    if (!fs::exists(trophy_path)) {
        try {
            fs::create_directories(trophy_path);
        } catch (const std::exception& e) {
            printf("[SceTrophy] Failed to create trophy directory: %s\n", e.what());
            handles.erase(handle.id);
            return SCE_TROPHY_ERROR_IO;
        }
    }
    
    // Load trophy configuration
    if (!load_trophy_conf(handle.id)) {
        printf("[SceTrophy] Warning: Failed to load trophy configuration for handle %d\n", handle.id);
        // We don't return an error here, as the trophy configuration might not exist yet
    }
    
    printf("[SceTrophy] Created handle %d for context %d, path: %s\n", 
           handle.id, context_id, handle.path.c_str());
    
    return SCE_TROPHY_ERROR_NONE;
}

// Destroy a trophy handle
int SceTrophyManager::destroy_handle(int handle_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    // Save trophy data before destroying the handle
    save_trophy_data(handle_id);
    
    // Remove trophy data for this handle
    trophy_data.erase(handle_id);
    
    // Remove the handle
    handles.erase(handle_id);
    
    printf("[SceTrophy] Destroyed handle %d\n", handle_id);
    
    return SCE_TROPHY_ERROR_NONE;
}

// Get trophy information
int SceTrophyManager::get_trophy_info(int handle_id, int trophy_id, SceTrophyInfo *info) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!info) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    if (!is_valid_trophy_id(trophy_id)) {
        return SCE_TROPHY_ERROR_INVALID_TROPHY_ID;
    }
    
    // TODO: Implement actual trophy info retrieval from TRPTITLE.DAT or similar
    // For now, we'll just return placeholder data
    
    info->trophy_id = trophy_id;
    info->trophy_grade = SCE_TROPHY_GRADE_BRONZE; // Default to bronze
    info->hidden = 0;
    
    snprintf(info->name, SCE_TROPHY_MAX_NAME_LENGTH, "Trophy %d", trophy_id);
    snprintf(info->description, SCE_TROPHY_MAX_DESC_LENGTH, "Description for Trophy %d", trophy_id);
    
    // Check if we have data for this trophy
    if (trophy_data.count(handle_id) > 0 && trophy_data[handle_id].count(trophy_id) > 0) {
        if (trophy_data[handle_id][trophy_id].unlocked) {
            info->hidden = 0; // If unlocked, it's not hidden anymore
        }
    }
    
    return SCE_TROPHY_ERROR_NONE;
}

// Get trophy group information
int SceTrophyManager::get_trophy_group_info(int handle_id, int group_id, SceTrophyGroupInfo *info) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!info) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    if (!is_valid_group_id(group_id)) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    // TODO: Implement actual trophy group info retrieval from TRPTITLE.DAT or similar
    // For now, we'll just return placeholder data
    
    info->group_id = group_id;
    
    snprintf(info->name, SCE_TROPHY_MAX_NAME_LENGTH, "Trophy Group %d", group_id);
    snprintf(info->description, SCE_TROPHY_MAX_DESC_LENGTH, "Description for Trophy Group %d", group_id);
    
    return SCE_TROPHY_ERROR_NONE;
}

// Get trophy count
int SceTrophyManager::get_trophy_count(int handle_id, int *platinum, int *gold, int *silver, int *bronze) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    // TODO: Implement actual trophy count retrieval from TRPTITLE.DAT or similar
    // For now, we'll just return placeholder data
    
    if (platinum) *platinum = 1;
    if (gold) *gold = 3;
    if (silver) *silver = 5;
    if (bronze) *bronze = 10;
    
    return SCE_TROPHY_ERROR_NONE;
}

// Get unlocked trophy count
int SceTrophyManager::get_trophy_unlocked_count(int handle_id, int *platinum, int *gold, int *silver, int *bronze) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    // Initialize counts
    int p_count = 0, g_count = 0, s_count = 0, b_count = 0;
    
    // Count unlocked trophies by grade
    if (trophy_data.count(handle_id) > 0) {
        for (const auto& trophy_pair : trophy_data[handle_id]) {
            if (trophy_pair.second.unlocked) {
                // TODO: Get actual grade from trophy info
                // For now, we'll just use a simple heuristic based on trophy ID
                int trophy_id = trophy_pair.first;
                if (trophy_id == 0) {
                    p_count++;
                } else if (trophy_id < 4) {
                    g_count++;
                } else if (trophy_id < 9) {
                    s_count++;
                } else {
                    b_count++;
                }
            }
        }
    }
    
    if (platinum) *platinum = p_count;
    if (gold) *gold = g_count;
    if (silver) *silver = s_count;
    if (bronze) *bronze = b_count;
    
    return SCE_TROPHY_ERROR_NONE;
}

// Unlock a trophy
int SceTrophyManager::unlock_trophy(int handle_id, int trophy_id, uint64_t *timestamp) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    if (!is_valid_trophy_id(trophy_id)) {
        return SCE_TROPHY_ERROR_INVALID_TROPHY_ID;
    }
    
    // Check if trophy is already unlocked
    if (trophy_data.count(handle_id) > 0 && 
        trophy_data[handle_id].count(trophy_id) > 0 && 
        trophy_data[handle_id][trophy_id].unlocked) {
        return SCE_TROPHY_ERROR_ALREADY_UNLOCKED;
    }
    
    // Cannot unlock platinum trophy directly
    if (trophy_id == 0) {
        return SCE_TROPHY_ERROR_CANNOT_UNLOCK_PLATINUM;
    }
    
    // Get current timestamp
    uint64_t current_time = static_cast<uint64_t>(time(nullptr));
    
    // Unlock the trophy
    SceTrophyData data;
    data.trophy_id = trophy_id;
    data.unlocked = 1;
    data.timestamp = current_time;
    
    trophy_data[handle_id][trophy_id] = data;
    
    // Save trophy data
    save_trophy_data(handle_id);
    
    // Return timestamp if requested
    if (timestamp) {
        *timestamp = current_time;
    }
    
    printf("[SceTrophy] Unlocked trophy %d for handle %d\n", trophy_id, handle_id);
    
    // TODO: Check if all trophies are unlocked and unlock platinum trophy if needed
    
    return SCE_TROPHY_ERROR_NONE;
}

// Get trophy icon
int SceTrophyManager::get_trophy_icon(int handle_id, int trophy_id, void *buffer, int *size) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!buffer || !size || *size <= 0) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    if (!is_valid_trophy_id(trophy_id)) {
        return SCE_TROPHY_ERROR_INVALID_TROPHY_ID;
    }
    
    // TODO: Implement actual trophy icon retrieval from TROP.SFM or similar
    // For now, we'll just return an error
    
    return SCE_TROPHY_ERROR_NOT_FOUND;
}

// Check if a trophy is unlocked
int SceTrophyManager::check_trophy_unlocked(int handle_id, int trophy_id, int *unlocked) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_TROPHY_ERROR_NOT_INITIALIZED;
    }
    
    if (!unlocked) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    if (!is_valid_handle(handle_id)) {
        return SCE_TROPHY_ERROR_INVALID_HANDLE;
    }
    
    if (!is_valid_trophy_id(trophy_id)) {
        return SCE_TROPHY_ERROR_INVALID_TROPHY_ID;
    }
    
    // Check if trophy is unlocked
    *unlocked = 0;
    if (trophy_data.count(handle_id) > 0 && 
        trophy_data[handle_id].count(trophy_id) > 0) {
        *unlocked = trophy_data[handle_id][trophy_id].unlocked;
    }
    
    return SCE_TROPHY_ERROR_NONE;
}

// Helper function to check if a context is valid
bool SceTrophyManager::is_valid_context(int context_id) const {
    return contexts.count(context_id) > 0 && contexts.at(context_id).initialized;
}

// Helper function to check if a handle is valid
bool SceTrophyManager::is_valid_handle(int handle_id) const {
    return handles.count(handle_id) > 0 && handles.at(handle_id).initialized;
}

// Helper function to check if a trophy ID is valid
bool SceTrophyManager::is_valid_trophy_id(int trophy_id) const {
    return trophy_id >= 0 && trophy_id < SCE_TROPHY_MAX_TROPHIES;
}

// Helper function to check if a group ID is valid
bool SceTrophyManager::is_valid_group_id(int group_id) const {
    return group_id >= 0 && group_id < SCE_TROPHY_MAX_GROUPS;
}

// Helper function to load trophy configuration
bool SceTrophyManager::load_trophy_conf(int handle_id) {
    if (!is_valid_handle(handle_id)) {
        return false;
    }
    
    const std::string& path = handles[handle_id].path;
    std::string conf_path = path + "/trophy_data.json";
    
    // Check if file exists
    if (!fs::exists(conf_path)) {
        return false;
    }
    
    try {
        // Open file
        std::ifstream file(conf_path);
        if (!file.is_open()) {
            printf("[SceTrophy] Failed to open trophy data file: %s\n", conf_path.c_str());
            return false;
        }
        
        // Parse JSON
        json data;
        file >> data;
        file.close();
        
        // Clear existing data
        trophy_data[handle_id].clear();
        
        // Load trophy data
        for (const auto& item : data["trophies"]) {
            SceTrophyData trophy;
            trophy.trophy_id = item["id"];
            trophy.unlocked = item["unlocked"];
            trophy.timestamp = item["timestamp"];
            
            trophy_data[handle_id][trophy.trophy_id] = trophy;
        }
        
        printf("[SceTrophy] Loaded trophy data for handle %d\n", handle_id);
        return true;
    } catch (const std::exception& e) {
        printf("[SceTrophy] Error loading trophy data: %s\n", e.what());
        return false;
    }
}

// Helper function to save trophy data
bool SceTrophyManager::save_trophy_data(int handle_id) {
    if (!is_valid_handle(handle_id)) {
        return false;
    }
    
    // If there's no trophy data, nothing to save
    if (trophy_data.count(handle_id) == 0 || trophy_data[handle_id].empty()) {
        return true;
    }
    
    const std::string& path = handles[handle_id].path;
    std::string conf_path = path + "/trophy_data.json";
    
    try {
        // Create JSON data
        json data;
        json trophies = json::array();
        
        for (const auto& trophy_pair : trophy_data[handle_id]) {
            const SceTrophyData& trophy = trophy_pair.second;
            json trophy_data;
            trophy_data["id"] = trophy.trophy_id;
            trophy_data["unlocked"] = trophy.unlocked;
            trophy_data["timestamp"] = trophy.timestamp;
            
            trophies.push_back(trophy_data);
        }
        
        data["trophies"] = trophies;
        
        // Write to file
        std::ofstream file(conf_path);
        if (!file.is_open()) {
            printf("[SceTrophy] Failed to open trophy data file for writing: %s\n", conf_path.c_str());
            return false;
        }
        
        file << data.dump(4); // Pretty print with 4-space indentation
        file.close();
        
        printf("[SceTrophy] Saved trophy data for handle %d\n", handle_id);
        return true;
    } catch (const std::exception& e) {
        printf("[SceTrophy] Error saving trophy data: %s\n", e.what());
        return false;
    }
}

// Helper function to get trophy path
std::string SceTrophyManager::get_trophy_path(int handle_id) const {
    if (!is_valid_handle(handle_id)) {
        return "";
    }
    
    return handles.at(handle_id).path;
}

// Module functions

// Initialize the trophy system
int sceTrophyInit() {
    return g_trophy_manager.initialize();
}

// Terminate the trophy system
int sceTrophyTerm() {
    return g_trophy_manager.terminate();
}

// Create a trophy context
int sceTrophyCreateContext(int *context, const char *comm_id, const char *comm_sign, int flags) {
    return g_trophy_manager.create_context(context, comm_id, comm_sign, flags);
}

// Destroy a trophy context
int sceTrophyDestroyContext(int context_id) {
    return g_trophy_manager.destroy_context(context_id);
}

// Create a trophy handle
int sceTrophyCreateHandle(int *handle, int context, const char *dir_name, int flags) {
    return g_trophy_manager.create_handle(handle, context, dir_name, flags);
}

// Destroy a trophy handle
int sceTrophyDestroyHandle(int handle_id) {
    return g_trophy_manager.destroy_handle(handle_id);
}

// Get trophy information
int sceTrophyGetTrophyInfo(int handle, int trophy_id, SceTrophyInfo *info) {
    return g_trophy_manager.get_trophy_info(handle, trophy_id, info);
}

// Get trophy group information
int sceTrophyGetTrophyGroupInfo(int handle, int group_id, SceTrophyGroupInfo *info) {
    return g_trophy_manager.get_trophy_group_info(handle, group_id, info);
}

// Get trophy count
int sceTrophyGetTrophyCount(int handle, int *platinum, int *gold, int *silver, int *bronze) {
    return g_trophy_manager.get_trophy_count(handle, platinum, gold, silver, bronze);
}

// Get trophy unlock state
int sceTrophyGetTrophyUnlockState(int handle, SceTrophyData *data, int count, uint32_t *unlocked) {
    if (!data || count <= 0 || !unlocked) {
        return SCE_TROPHY_ERROR_INVALID_PARAM;
    }
    
    *unlocked = 0;
    
    for (int i = 0; i < count; i++) {
        int trophy_unlocked = 0;
        int ret = g_trophy_manager.check_trophy_unlocked(handle, data[i].trophy_id, &trophy_unlocked);
        if (ret != SCE_TROPHY_ERROR_NONE) {
            return ret;
        }
        
        if (trophy_unlocked) {
            data[i].unlocked = 1;
            data[i].timestamp = 0; // We don't return timestamp here
            (*unlocked)++;
        } else {
            data[i].unlocked = 0;
            data[i].timestamp = 0;
        }
    }
    
    return SCE_TROPHY_ERROR_NONE;
}

// Unlock a trophy
int sceTrophyUnlockTrophy(int handle, int trophy_id, uint64_t *timestamp) {
    return g_trophy_manager.unlock_trophy(handle, trophy_id, timestamp);
}

// Get trophy icon
int sceTrophyGetTrophyIcon(int handle, int trophy_id, void *buffer, int *size) {
    return g_trophy_manager.get_trophy_icon(handle, trophy_id, buffer, size);
}

// Get trophy unlock state (simplified version)
int sceTrophyGetTrophyUnlockState2(int handle, int trophy_id, int *unlocked) {
    return g_trophy_manager.check_trophy_unlocked(handle, trophy_id, unlocked);
}

} // namespace trophy
} // namespace modules
} // namespace firmware
