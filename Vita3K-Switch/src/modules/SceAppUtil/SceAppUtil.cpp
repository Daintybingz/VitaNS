#include "SceAppUtil.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>

namespace fs = std::filesystem;

SceAppUtil::SceAppUtil()
    : Module("SceAppUtil")
    , initialized(false)
    , saveDataBasePath("saves")
{
    // Register system calls
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0xE1878282, (void*)&SceAppUtil::sceAppUtilInit);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x9C70C057, (void*)&SceAppUtil::sceAppUtilShutdown);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0xFE8A0B46, (void*)&SceAppUtil::sceAppUtilSaveDataSlotCreate);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x98630136, (void*)&SceAppUtil::sceAppUtilSaveDataSlotDelete);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0xA12D7C79, (void*)&SceAppUtil::sceAppUtilSaveDataSlotGetParam);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0xF3D65520, (void*)&SceAppUtil::sceAppUtilSaveDataSlotSetParam);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0xD4C9B69A, (void*)&SceAppUtil::sceAppUtilSaveDataSlotSearch);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x96A42BAE, (void*)&SceAppUtil::sceAppUtilSaveDataMount);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x32E32DCB, (void*)&SceAppUtil::sceAppUtilSaveDataUmount);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0xF3E81219, (void*)&SceAppUtil::sceAppUtilSaveDataGetSlotList);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x34EAEE6F, (void*)&SceAppUtil::sceAppUtilSaveDataDataSave);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x87C67B27, (void*)&SceAppUtil::sceAppUtilSaveDataDataLoad);
    // TODO: Refactor to register static or bound member function for syscall registration
    // registerFunction(0x9C70C057, (void*)&SceAppUtil::sceAppUtilSaveDataDataRemove);
}

SceAppUtil::~SceAppUtil() {
    finalize();
}

bool SceAppUtil::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        return true;
    }
    
    printf("[SceAppUtil] Initializing AppUtil module\n");
    
    // Create save data base directory if it doesn't exist
    if (!fs::exists(saveDataBasePath)) {
        try {
            fs::create_directories(saveDataBasePath);
        } catch (const std::exception& e) {
            printf("[SceAppUtil] Error creating save data directory: %s\n", e.what());
            return false;
        }
    }
    
    initialized = true;
    printf("[SceAppUtil] AppUtil module initialized successfully\n");
    
    return true;
}

void SceAppUtil::finalize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return;
    }
    
    printf("[SceAppUtil] Finalizing AppUtil module\n");
    
    // Unmount all mounted save data
    for (const auto& mountPoint : mountedPoints) {
        printf("[SceAppUtil] Auto-unmounting save data: %s\n", mountPoint.c_str());
    }
    mountedPoints.clear();
    
    // Clear save data slots
    saveDataSlots.clear();
    
    initialized = false;
    printf("[SceAppUtil] AppUtil module finalized\n");
}

// AppUtil system call implementations
int SceAppUtil::sceAppUtilInit(const SceAppUtilInitParam* initParam, const SceAppUtilBootParam* bootParam) {
    printf("[SceAppUtil] sceAppUtilInit called\n");
    
    if (!initParam || !bootParam) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Initialize the module
    if (!initialize()) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    return 0;
}

int SceAppUtil::sceAppUtilShutdown() {
    printf("[SceAppUtil] sceAppUtilShutdown called\n");
    
    // Finalize the module
    finalize();
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataSlotCreate(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param) {
    printf("[SceAppUtil] sceAppUtilSaveDataSlotCreate called: slotId=%d\n", slotId);
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!param) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Check if slot already exists
    if (saveDataSlots.find(slotId) != saveDataSlots.end()) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_EXISTS;
    }
    
    // Create a new save data slot
    SaveDataSlot slot;
    slot.id = std::string(param->id, strnlen(param->id, SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE));
    slot.titleId = std::string(param->titleId, strnlen(param->titleId, SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE));
    slot.dirName = std::string(param->dirName, strnlen(param->dirName, SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE));
    slot.title = std::string(param->title, strnlen(param->title, SCE_APPUTIL_SAVEDATA_TITLE_SIZE));
    slot.userParam = param->userParam;
    slot.size = 0;
    slot.mounted = false;
    
    // Create save data directory
    slot.path = getSaveDataPath(slot.titleId, slot.dirName);
    if (!createSaveDataDirectory(slot.path)) {
        return SCE_APPUTIL_ERROR_FILE_IO;
    }
    
    // Store the slot
    saveDataSlots[slotId] = slot;
    
    printf("[SceAppUtil] Created save data slot: %s (path: %s)\n", slot.id.c_str(), slot.path.c_str());
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataSlotDelete(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param) {
    printf("[SceAppUtil] sceAppUtilSaveDataSlotDelete called: slotId=%d\n", slotId);
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    // Check if slot exists
    auto it = saveDataSlots.find(slotId);
    if (it == saveDataSlots.end()) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND;
    }
    
    // Check if slot is mounted
    if (it->second.mounted) {
        return SCE_APPUTIL_ERROR_BUSY;
    }
    
    // Remove save data directory
    if (!removeSaveDataDirectory(it->second.path)) {
        return SCE_APPUTIL_ERROR_FILE_IO;
    }
    
    // Remove the slot
    saveDataSlots.erase(it);
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataSlotGetParam(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param, SceAppUtilSaveDataSlotParam* outParam) {
    printf("[SceAppUtil] sceAppUtilSaveDataSlotGetParam called: slotId=%d\n", slotId);
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!outParam) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Check if slot exists
    auto it = saveDataSlots.find(slotId);
    if (it == saveDataSlots.end()) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND;
    }
    
    // Copy slot parameters to output
    const SaveDataSlot& slot = it->second;
    
    strncpy(outParam->id, slot.id.c_str(), SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE - 1);
    outParam->id[SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE - 1] = '\0';
    
    strncpy(outParam->titleId, slot.titleId.c_str(), SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE - 1);
    outParam->titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE - 1] = '\0';
    
    strncpy(outParam->dirName, slot.dirName.c_str(), SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE - 1);
    outParam->dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE - 1] = '\0';
    
    strncpy(outParam->title, slot.title.c_str(), SCE_APPUTIL_SAVEDATA_TITLE_SIZE - 1);
    outParam->title[SCE_APPUTIL_SAVEDATA_TITLE_SIZE - 1] = '\0';
    
    outParam->userParam = slot.userParam;
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataSlotSetParam(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param) {
    printf("[SceAppUtil] sceAppUtilSaveDataSlotSetParam called: slotId=%d\n", slotId);
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!param) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Check if slot exists
    auto it = saveDataSlots.find(slotId);
    if (it == saveDataSlots.end()) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND;
    }
    
    // Update slot parameters
    SaveDataSlot& slot = it->second;
    
    slot.id = std::string(param->id, strnlen(param->id, SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE));
    slot.titleId = std::string(param->titleId, strnlen(param->titleId, SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE));
    slot.dirName = std::string(param->dirName, strnlen(param->dirName, SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE));
    slot.title = std::string(param->title, strnlen(param->title, SCE_APPUTIL_SAVEDATA_TITLE_SIZE));
    slot.userParam = param->userParam;
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataSlotSearch(const SceAppUtilSaveDataSlotSearchCond* cond, SceAppUtilSaveDataSlotSearchResult* result, SceAppUtilSaveDataSlotParam* outParam) {
    printf("[SceAppUtil] sceAppUtilSaveDataSlotSearch called\n");
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!cond || !result || !outParam) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Search for slots matching the condition
    int slotCount = 0;
    
    for (const auto& pair : saveDataSlots) {
        const SaveDataSlot& slot = pair.second;
        
        // Check if slot matches the search condition
        if (cond->on) {
            std::string searchTitleId = std::string(cond->titleId, strnlen(cond->titleId, SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE));
            std::string searchDirName = std::string(cond->dirName, strnlen(cond->dirName, SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE));
            
            if (!searchTitleId.empty() && slot.titleId != searchTitleId) {
                continue;
            }
            
            if (!searchDirName.empty() && slot.dirName != searchDirName) {
                continue;
            }
        }
        
        // Found a matching slot
        if (slotCount == 0) {
            // Copy the first matching slot to outParam
            strncpy(outParam->id, slot.id.c_str(), SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE - 1);
            outParam->id[SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE - 1] = '\0';
            
            strncpy(outParam->titleId, slot.titleId.c_str(), SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE - 1);
            outParam->titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE - 1] = '\0';
            
            strncpy(outParam->dirName, slot.dirName.c_str(), SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE - 1);
            outParam->dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE - 1] = '\0';
            
            strncpy(outParam->title, slot.title.c_str(), SCE_APPUTIL_SAVEDATA_TITLE_SIZE - 1);
            outParam->title[SCE_APPUTIL_SAVEDATA_TITLE_SIZE - 1] = '\0';
            
            outParam->userParam = slot.userParam;
        }
        
        slotCount++;
    }
    
    // Set the result
    result->slotCount = slotCount;
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataMount(const SceAppUtilSaveDataMountPoint* mountPoint, const SceAppUtilSaveDataMountInfo* mountInfo) {
    printf("[SceAppUtil] sceAppUtilSaveDataMount called\n");
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!mountPoint) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Check if we've reached the maximum number of mounted points
    if (mountedPoints.size() >= SCE_APPUTIL_SAVEDATA_MAX_MOUNT) {
        return SCE_APPUTIL_ERROR_BUSY;
    }
    
    // Get the mount point name
    std::string mountPointName = std::string(mountPoint->data, strnlen(mountPoint->data, SCE_APPUTIL_SAVEDATA_MOUNT_POINT_DATA_MAXSIZE));
    
    // Check if the mount point is already mounted
    for (const auto& mp : mountedPoints) {
        if (mp == mountPointName) {
            return SCE_APPUTIL_ERROR_BUSY;
        }
    }
    
    // Add the mount point to the list
    mountedPoints.push_back(mountPointName);
    
    // Mark the corresponding slot as mounted
    for (auto& pair : saveDataSlots) {
        SaveDataSlot& slot = pair.second;
        if (slot.path.find(mountPointName) != std::string::npos) {
            slot.mounted = true;
            break;
        }
    }
    
    printf("[SceAppUtil] Mounted save data: %s\n", mountPointName.c_str());
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataUmount(const SceAppUtilSaveDataMountPoint* mountPoint) {
    printf("[SceAppUtil] sceAppUtilSaveDataUmount called\n");
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!mountPoint) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Get the mount point name
    std::string mountPointName = std::string(mountPoint->data, strnlen(mountPoint->data, SCE_APPUTIL_SAVEDATA_MOUNT_POINT_DATA_MAXSIZE));
    
    // Check if the mount point is mounted
    auto it = std::find(mountedPoints.begin(), mountedPoints.end(), mountPointName);
    if (it == mountedPoints.end()) {
        return SCE_APPUTIL_ERROR_NOT_MOUNTED;
    }
    
    // Remove the mount point from the list
    mountedPoints.erase(it);
    
    // Mark the corresponding slot as unmounted
    for (auto& pair : saveDataSlots) {
        SaveDataSlot& slot = pair.second;
        if (slot.path.find(mountPointName) != std::string::npos) {
            slot.mounted = false;
            break;
        }
    }
    
    printf("[SceAppUtil] Unmounted save data: %s\n", mountPointName.c_str());
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataGetSlotList(SceAppUtilSaveDataSlotParam* param, int32_t* slotNum) {
    printf("[SceAppUtil] sceAppUtilSaveDataGetSlotList called\n");
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!param || !slotNum || *slotNum <= 0) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Get the number of slots
    int32_t numSlots = std::min(static_cast<int32_t>(saveDataSlots.size()), *slotNum);
    
    // Copy slot parameters to output
    int32_t i = 0;
    for (const auto& pair : saveDataSlots) {
        if (i >= numSlots) {
            break;
        }
        
        const SaveDataSlot& slot = pair.second;
        
        strncpy(param[i].id, slot.id.c_str(), SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE - 1);
        param[i].id[SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE - 1] = '\0';
        
        strncpy(param[i].titleId, slot.titleId.c_str(), SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE - 1);
        param[i].titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE - 1] = '\0';
        
        strncpy(param[i].dirName, slot.dirName.c_str(), SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE - 1);
        param[i].dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE - 1] = '\0';
        
        strncpy(param[i].title, slot.title.c_str(), SCE_APPUTIL_SAVEDATA_TITLE_SIZE - 1);
        param[i].title[SCE_APPUTIL_SAVEDATA_TITLE_SIZE - 1] = '\0';
        
        param[i].userParam = slot.userParam;
        
        i++;
    }
    
    // Set the number of slots
    *slotNum = numSlots;
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataDataSave(const SceAppUtilSaveDataFileSlot* slot, const void* buf, uint32_t bufSize, const SceAppUtilSaveDataMountPoint* mountPoint) {
    printf("[SceAppUtil] sceAppUtilSaveDataDataSave called: bufSize=%d\n", bufSize);
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!slot || !buf || !mountPoint) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Get the mount point name
    std::string mountPointName = std::string(mountPoint->data, strnlen(mountPoint->data, SCE_APPUTIL_SAVEDATA_MOUNT_POINT_DATA_MAXSIZE));
    
    // Check if the mount point is mounted
    auto it = std::find(mountedPoints.begin(), mountedPoints.end(), mountPointName);
    if (it == mountedPoints.end()) {
        return SCE_APPUTIL_ERROR_NOT_MOUNTED;
    }
    
    // Find the corresponding slot
    SaveDataSlot* targetSlot = nullptr;
    for (auto& pair : saveDataSlots) {
        SaveDataSlot& s = pair.second;
        if (s.path.find(mountPointName) != std::string::npos) {
            targetSlot = &s;
            break;
        }
    }
    
    if (!targetSlot) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND;
    }
    
    // Create the file path
    std::string filePath = targetSlot->path + "/data_" + std::to_string(slot->id);
    
    // Save the data to file
    std::ofstream file(filePath, std::ios::binary);
    if (!file) {
        return SCE_APPUTIL_ERROR_FILE_IO;
    }
    
    file.write(static_cast<const char*>(buf), bufSize);
    file.close();
    
    // Update slot size
    targetSlot->size += bufSize;
    
    printf("[SceAppUtil] Saved data to file: %s (size: %d bytes)\n", filePath.c_str(), bufSize);
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataDataLoad(const SceAppUtilSaveDataFileSlot* slot, void* buf, uint32_t bufSize, const SceAppUtilSaveDataMountPoint* mountPoint) {
    printf("[SceAppUtil] sceAppUtilSaveDataDataLoad called: bufSize=%d\n", bufSize);
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!slot || !buf || !mountPoint) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Get the mount point name
    std::string mountPointName = std::string(mountPoint->data, strnlen(mountPoint->data, SCE_APPUTIL_SAVEDATA_MOUNT_POINT_DATA_MAXSIZE));
    
    // Check if the mount point is mounted
    auto it = std::find(mountedPoints.begin(), mountedPoints.end(), mountPointName);
    if (it == mountedPoints.end()) {
        return SCE_APPUTIL_ERROR_NOT_MOUNTED;
    }
    
    // Find the corresponding slot
    SaveDataSlot* targetSlot = nullptr;
    for (auto& pair : saveDataSlots) {
        SaveDataSlot& s = pair.second;
        if (s.path.find(mountPointName) != std::string::npos) {
            targetSlot = &s;
            break;
        }
    }
    
    if (!targetSlot) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND;
    }
    
    // Create the file path
    std::string filePath = targetSlot->path + "/data_" + std::to_string(slot->id);
    
    // Check if the file exists
    if (!fs::exists(filePath)) {
        return SCE_APPUTIL_ERROR_FILE_IO;
    }
    
    // Load the data from file
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return SCE_APPUTIL_ERROR_FILE_IO;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // Check if buffer is large enough
    if (fileSize > bufSize) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Read the data
    file.read(static_cast<char*>(buf), fileSize);
    file.close();
    
    printf("[SceAppUtil] Loaded data from file: %s (size: %ld bytes)\n", filePath.c_str(), fileSize);
    
    return 0;
}

int SceAppUtil::sceAppUtilSaveDataDataRemove(const SceAppUtilSaveDataRemoveItem* item) {
    printf("[SceAppUtil] sceAppUtilSaveDataDataRemove called\n");
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_APPUTIL_ERROR_NOT_INITIALIZED;
    }
    
    if (!item) {
        return SCE_APPUTIL_ERROR_PARAMETER;
    }
    
    // Get title ID and directory name
    std::string titleId = std::string(item->titleId, strnlen(item->titleId, SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE));
    std::string dirName = std::string(item->dirName, strnlen(item->dirName, SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE));
    
    // Find the corresponding slot
    SaveDataSlot* targetSlot = nullptr;
    uint32_t targetSlotId = 0;
    
    for (auto& pair : saveDataSlots) {
        SaveDataSlot& slot = pair.second;
        if (slot.titleId == titleId && slot.dirName == dirName) {
            targetSlot = &slot;
            targetSlotId = pair.first;
            break;
        }
    }
    
    if (!targetSlot) {
        return SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND;
    }
    
    // Check if slot is mounted
    if (targetSlot->mounted) {
        return SCE_APPUTIL_ERROR_BUSY;
    }
    
    // Remove save data based on mode
    if (item->mode == SCE_APPUTIL_SAVEDATA_DATA_REMOVE_MODE_DEFAULT) {
        // Remove all files in the directory
        for (const auto& entry : fs::directory_iterator(targetSlot->path)) {
            fs::remove_all(entry.path());
        }
    } else if (item->mode == SCE_APPUTIL_SAVEDATA_DATA_REMOVE_MODE_NO_SLOT) {
        // Remove the entire slot
        if (!removeSaveDataDirectory(targetSlot->path)) {
            return SCE_APPUTIL_ERROR_FILE_IO;
        }
        
        // Remove the slot from the map
        saveDataSlots.erase(targetSlotId);
    }
    
    return 0;
}

// Helper functions
std::string SceAppUtil::getSaveDataPath(const std::string& titleId, const std::string& dirName) {
    return saveDataBasePath + "/" + titleId + "/" + dirName;
}

bool SceAppUtil::createSaveDataDirectory(const std::string& path) {
    try {
        fs::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        printf("[SceAppUtil] Error creating directory: %s - %s\n", path.c_str(), e.what());
        return false;
    }
}

bool SceAppUtil::removeSaveDataDirectory(const std::string& path) {
    try {
        fs::remove_all(path);
        return true;
    } catch (const std::exception& e) {
        printf("[SceAppUtil] Error removing directory: %s - %s\n", path.c_str(), e.what());
        return false;
    }
}
