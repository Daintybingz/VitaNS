#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace firmware {
namespace modules {
namespace trophy {

// Trophy error codes
enum SceTrophyErrorCode {
    SCE_TROPHY_ERROR_NONE                     = 0,
    SCE_TROPHY_ERROR_NOT_INITIALIZED          = 0x80021001,
    SCE_TROPHY_ERROR_ALREADY_INITIALIZED      = 0x80021002,
    SCE_TROPHY_ERROR_INVALID_CONTEXT          = 0x80021003,
    SCE_TROPHY_ERROR_INVALID_HANDLE           = 0x80021004,
    SCE_TROPHY_ERROR_INVALID_TROPHY_ID        = 0x80021005,
    SCE_TROPHY_ERROR_INVALID_TROPHY_GRADE     = 0x80021006,
    SCE_TROPHY_ERROR_INVALID_TROPHY_FLAG      = 0x80021007,
    SCE_TROPHY_ERROR_INVALID_NP_COMM_ID       = 0x80021008,
    SCE_TROPHY_ERROR_INVALID_NP_COMM_SIGN     = 0x80021009,
    SCE_TROPHY_ERROR_INVALID_BUFFER           = 0x8002100A,
    SCE_TROPHY_ERROR_INVALID_TYPE             = 0x8002100B,
    SCE_TROPHY_ERROR_INVALID_PARAM            = 0x8002100C,
    SCE_TROPHY_ERROR_INVALID_ICON_DATA        = 0x8002100D,
    SCE_TROPHY_ERROR_INSUFFICIENT             = 0x8002100E,
    SCE_TROPHY_ERROR_ABORT                    = 0x8002100F,
    SCE_TROPHY_ERROR_UNKNOWN_FORMAT           = 0x80021010,
    SCE_TROPHY_ERROR_NOT_FOUND                = 0x80021011,
    SCE_TROPHY_ERROR_ALREADY_UNLOCKED         = 0x80021012,
    SCE_TROPHY_ERROR_LOCKED                   = 0x80021013,
    SCE_TROPHY_ERROR_HIDDEN                   = 0x80021014,
    SCE_TROPHY_ERROR_CANNOT_UNLOCK_PLATINUM   = 0x80021015,
    SCE_TROPHY_ERROR_ALREADY_EXISTS           = 0x80021016,
    SCE_TROPHY_ERROR_NO_TROPHY_ENTRY          = 0x80021017,
    SCE_TROPHY_ERROR_INVALID_TYPE_FOR_CONTEXT = 0x80021018,
    SCE_TROPHY_ERROR_FOLDER_FULL              = 0x80021019,
    SCE_TROPHY_ERROR_IO                       = 0x8002101A,
    SCE_TROPHY_ERROR_UNSUPPORTED_FORMAT       = 0x8002101B,
    SCE_TROPHY_ERROR_INSUFFICIENT_DISK_SPACE  = 0x8002101C,
    SCE_TROPHY_ERROR_INVALID_CONTEXT_ID       = 0x8002101D,
    SCE_TROPHY_ERROR_INVALID_FILE             = 0x8002101E,
    SCE_TROPHY_ERROR_INVALID_DISC_ID          = 0x8002101F,
    SCE_TROPHY_ERROR_INVALID_ON_DISC_ID       = 0x80021020,
    SCE_TROPHY_ERROR_INVALID_TROPHY_CONF      = 0x80021021,
    SCE_TROPHY_ERROR_SETUP_REQUIRED           = 0x80021022,
    SCE_TROPHY_ERROR_NETWORKING               = 0x80021023,
    SCE_TROPHY_ERROR_NEED_LOGIN               = 0x80021024,
    SCE_TROPHY_ERROR_ALREADY_REGISTERED       = 0x80021025,
    SCE_TROPHY_ERROR_FATAL                    = 0x800210FF
};

// Trophy constants
#define SCE_TROPHY_MAX_TROPHIES          128
#define SCE_TROPHY_MAX_GROUPS            16
#define SCE_TROPHY_MAX_NAME_LENGTH       128
#define SCE_TROPHY_MAX_DESC_LENGTH       1024
#define SCE_TROPHY_MAX_ICON_SIZE         (128 * 1024)

// Trophy grades
enum SceTrophyGrade {
    SCE_TROPHY_GRADE_UNKNOWN     = 0,
    SCE_TROPHY_GRADE_PLATINUM    = 1,
    SCE_TROPHY_GRADE_GOLD        = 2,
    SCE_TROPHY_GRADE_SILVER      = 3,
    SCE_TROPHY_GRADE_BRONZE      = 4
};

// Trophy flags
enum SceTrophyFlag {
    SCE_TROPHY_FLAG_HIDDEN       = 0x1,
    SCE_TROPHY_FLAG_UNLOCKED     = 0x2,
    SCE_TROPHY_FLAG_INVALID      = 0x4,
    SCE_TROPHY_FLAG_LOCKED       = 0x8
};

// Trophy context flags
enum SceTrophyContextFlag {
    SCE_TROPHY_CONTEXT_FLAG_NONE         = 0,
    SCE_TROPHY_CONTEXT_FLAG_READ_ONLY    = 1
};

// Trophy structures
struct SceTrophyContext {
    int id;
    std::string comm_id;
    std::string comm_sign;
    int flags;
    bool initialized;
};

struct SceTrophyHandle {
    int id;
    int context_id;
    std::string path;
    bool initialized;
};

struct SceTrophyInfo {
    uint32_t trophy_id;
    uint32_t trophy_grade;
    uint32_t hidden;
    char name[SCE_TROPHY_MAX_NAME_LENGTH];
    char description[SCE_TROPHY_MAX_DESC_LENGTH];
};

struct SceTrophyGroupInfo {
    uint32_t group_id;
    char name[SCE_TROPHY_MAX_NAME_LENGTH];
    char description[SCE_TROPHY_MAX_DESC_LENGTH];
};

struct SceTrophyData {
    uint32_t trophy_id;
    uint32_t unlocked;
    uint64_t timestamp;
};

// Trophy manager class
class SceTrophyManager {
public:
    SceTrophyManager();
    ~SceTrophyManager();

    // Initialize the trophy system
    int initialize();

    // Terminate the trophy system
    int terminate();

    // Trophy context functions
    int create_context(int *context_id, const char *comm_id, const char *comm_sign, int flags);
    int destroy_context(int context_id);
    
    // Trophy handle functions
    int create_handle(int *handle_id, int context_id, const char *dir_name, int flags);
    int destroy_handle(int handle_id);
    
    // Trophy functions
    int get_trophy_info(int handle_id, int trophy_id, SceTrophyInfo *info);
    int get_trophy_group_info(int handle_id, int group_id, SceTrophyGroupInfo *info);
    int get_trophy_count(int handle_id, int *platinum, int *gold, int *silver, int *bronze);
    int get_trophy_unlocked_count(int handle_id, int *platinum, int *gold, int *silver, int *bronze);
    int unlock_trophy(int handle_id, int trophy_id, uint64_t *timestamp);
    int get_trophy_icon(int handle_id, int trophy_id, void *buffer, int *size);
    int check_trophy_unlocked(int handle_id, int trophy_id, int *unlocked);

private:
    // Trophy initialized flag
    bool initialized;

    // Trophy contexts
    std::map<int, SceTrophyContext> contexts;

    // Trophy handles
    std::map<int, SceTrophyHandle> handles;

    // Trophy data (handle_id -> trophy_id -> data)
    std::map<int, std::map<int, SceTrophyData>> trophy_data;

    // Next context ID
    int next_context_id;

    // Next handle ID
    int next_handle_id;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Helper functions
    bool is_valid_context(int context_id) const;
    bool is_valid_handle(int handle_id) const;
    bool is_valid_trophy_id(int trophy_id) const;
    bool is_valid_group_id(int group_id) const;
    bool load_trophy_conf(int handle_id);
    bool save_trophy_data(int handle_id);
    std::string get_trophy_path(int handle_id) const;
};

// Module functions
int sceTrophyInit();
int sceTrophyTerm();

// Context functions
int sceTrophyCreateContext(int *context, const char *comm_id, const char *comm_sign, int flags);
int sceTrophyDestroyContext(int context_id);

// Handle functions
int sceTrophyCreateHandle(int *handle, int context, const char *dir_name, int flags);
int sceTrophyDestroyHandle(int handle_id);

// Trophy functions
int sceTrophyGetTrophyInfo(int handle, int trophy_id, SceTrophyInfo *info);
int sceTrophyGetTrophyGroupInfo(int handle, int group_id, SceTrophyGroupInfo *info);
int sceTrophyGetTrophyCount(int handle, int *platinum, int *gold, int *silver, int *bronze);
int sceTrophyGetTrophyUnlockState(int handle, SceTrophyData *data, int count, uint32_t *unlocked);
int sceTrophyUnlockTrophy(int handle, int trophy_id, uint64_t *timestamp);
int sceTrophyGetTrophyIcon(int handle, int trophy_id, void *buffer, int *size);
int sceTrophyGetTrophyUnlockState2(int handle, int trophy_id, int *unlocked);

} // namespace trophy
} // namespace modules
} // namespace firmware
