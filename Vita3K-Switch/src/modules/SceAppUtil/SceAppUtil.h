#pragma once

#include "../module.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

// AppUtil error codes
#define SCE_APPUTIL_ERROR_PARAMETER          0x80100600
#define SCE_APPUTIL_ERROR_NOT_INITIALIZED    0x80100601
#define SCE_APPUTIL_ERROR_NO_MEMORY          0x80100602
#define SCE_APPUTIL_ERROR_BUSY               0x80100603
#define SCE_APPUTIL_ERROR_NOT_MOUNTED        0x80100604
#define SCE_APPUTIL_ERROR_NO_PERMISSION      0x80100605
#define SCE_APPUTIL_ERROR_FILE_IO            0x80100606
#define SCE_APPUTIL_ERROR_SAVEDATA_SLOT_EXISTS 0x80100620
#define SCE_APPUTIL_ERROR_SAVEDATA_SLOT_NOT_FOUND 0x80100621
#define SCE_APPUTIL_ERROR_SAVEDATA_NO_SPACE_QUOTA 0x80100622
#define SCE_APPUTIL_ERROR_SAVEDATA_NO_SPACE_FS 0x80100623

// AppUtil constants
#define SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE 32
#define SCE_APPUTIL_SAVEDATA_DATA_REMOVE_MODE_DEFAULT 0
#define SCE_APPUTIL_SAVEDATA_DATA_REMOVE_MODE_NO_SLOT 1
#define SCE_APPUTIL_SAVEDATA_MAX_MOUNT 8
#define SCE_APPUTIL_SAVEDATA_ATTRIBUTE_SIZE 256
#define SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE 16
#define SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE 32
#define SCE_APPUTIL_SAVEDATA_TITLE_SIZE 128
#define SCE_APPUTIL_SAVEDATA_MOUNT_POINT_DATA_MAXSIZE 16

// AppUtil initialization parameters
typedef struct SceAppUtilInitParam {
    uint32_t workBufSize;
    char reserved[60];
} SceAppUtilInitParam;

// AppUtil boot parameters
typedef struct SceAppUtilBootParam {
    uint32_t attr;
    uint32_t appVersion;
    char reserved[32];
} SceAppUtilBootParam;

// AppUtil save data slot information
typedef struct SceAppUtilSaveDataSlotParam {
    char id[SCE_APPUTIL_SAVEDATA_SLOT_ID_SIZE];
    char titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE];
    char dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE];
    char title[SCE_APPUTIL_SAVEDATA_TITLE_SIZE];
    uint32_t iconBufSize;
    void* iconBuf;
    uint32_t userParam;
    char reserved[4];
} SceAppUtilSaveDataSlotParam;

// AppUtil save data information
typedef struct SceAppUtilSaveDataInfo {
    int32_t slotId;
    int32_t status;
    int32_t hddFreeSize;
    int32_t sizeKiB;
    char title[SCE_APPUTIL_SAVEDATA_TITLE_SIZE];
    char dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE];
    char reserved[64];
} SceAppUtilSaveDataInfo;

// AppUtil save data mount information
typedef struct SceAppUtilSaveDataMountPoint {
    char data[SCE_APPUTIL_SAVEDATA_MOUNT_POINT_DATA_MAXSIZE];
} SceAppUtilSaveDataMountPoint;

// AppUtil save data mount information
typedef struct SceAppUtilSaveDataSlotEmptyParam {
    char titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE];
    char dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE];
    char reserved[32];
} SceAppUtilSaveDataSlotEmptyParam;

// AppUtil save data remove mode
typedef struct SceAppUtilSaveDataRemoveItem {
    int32_t mode;
    char titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE];
    char dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE];
    char reserved[28];
} SceAppUtilSaveDataRemoveItem;

// AppUtil save data mount information
typedef struct SceAppUtilSaveDataMountInfo {
    int32_t maxFile;
    int32_t maxSize;
    int32_t systemSize;
    char reserved[36];
} SceAppUtilSaveDataMountInfo;

// AppUtil save data file system information
typedef struct SceAppUtilSaveDataFileSlot {
    uint32_t id;
    uint32_t format;
    char reserved[32];
} SceAppUtilSaveDataFileSlot;

// AppUtil save data file system information
typedef struct SceAppUtilSaveDataSlotSearchCond {
    uint8_t on;
    uint8_t reserved[3];
    char titleId[SCE_APPUTIL_SAVEDATA_TITLE_ID_SIZE];
    char dirName[SCE_APPUTIL_SAVEDATA_DIRNAME_SIZE];
    char reserved2[32];
} SceAppUtilSaveDataSlotSearchCond;

// AppUtil save data file system information
typedef struct SceAppUtilSaveDataSlotSearchResult {
    int32_t slotCount;
    char reserved[32];
} SceAppUtilSaveDataSlotSearchResult;

// Save data slot structure for internal use
struct SaveDataSlot {
    std::string id;
    std::string titleId;
    std::string dirName;
    std::string title;
    std::string path;
    uint32_t userParam;
    uint32_t size;
    bool mounted;
};

// SceAppUtil module implementation
class SceAppUtil : public Module {
public:
    SceAppUtil();
    ~SceAppUtil();
    
    // Initialize and finalize
    bool initialize();
    void finalize();
    
    // AppUtil system calls
    int sceAppUtilInit(const SceAppUtilInitParam* initParam, const SceAppUtilBootParam* bootParam);
    int sceAppUtilShutdown();
    int sceAppUtilSaveDataSlotCreate(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param);
    int sceAppUtilSaveDataSlotDelete(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param);
    int sceAppUtilSaveDataSlotGetParam(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param, SceAppUtilSaveDataSlotParam* outParam);
    int sceAppUtilSaveDataSlotSetParam(uint32_t slotId, const SceAppUtilSaveDataSlotParam* param);
    int sceAppUtilSaveDataSlotSearch(const SceAppUtilSaveDataSlotSearchCond* cond, SceAppUtilSaveDataSlotSearchResult* result, SceAppUtilSaveDataSlotParam* outParam);
    int sceAppUtilSaveDataMount(const SceAppUtilSaveDataMountPoint* mountPoint, const SceAppUtilSaveDataMountInfo* mountInfo);
    int sceAppUtilSaveDataUmount(const SceAppUtilSaveDataMountPoint* mountPoint);
    int sceAppUtilSaveDataGetSlotList(SceAppUtilSaveDataSlotParam* param, int32_t* slotNum);
    int sceAppUtilSaveDataDataSave(const SceAppUtilSaveDataFileSlot* slot, const void* buf, uint32_t bufSize, const SceAppUtilSaveDataMountPoint* mountPoint);
    int sceAppUtilSaveDataDataLoad(const SceAppUtilSaveDataFileSlot* slot, void* buf, uint32_t bufSize, const SceAppUtilSaveDataMountPoint* mountPoint);
    int sceAppUtilSaveDataDataRemove(const SceAppUtilSaveDataRemoveItem* item);
    
private:
    bool initialized;
    std::string saveDataBasePath;
    std::unordered_map<uint32_t, SaveDataSlot> saveDataSlots;
    std::vector<std::string> mountedPoints;
    std::mutex mutex;
    
    // Helper functions
    std::string getSaveDataPath(const std::string& titleId, const std::string& dirName);
    bool createSaveDataDirectory(const std::string& path);
    bool removeSaveDataDirectory(const std::string& path);
};
