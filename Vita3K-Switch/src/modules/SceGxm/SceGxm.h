#pragma once

#include "../module.h"
#include "../../core/emulator/emulator.h"
#include <GLES2/gl2.h>
#include <SDL2/SDL_opengles2.h>

namespace vita {
    // GXM Error Codes
    enum SceGxmErrorCode {
        SCE_GXM_ERROR_OK = 0,
        SCE_GXM_ERROR_INVALID_POINTER = 0x80020001,
        SCE_GXM_ERROR_INVALID_CONTEXT = 0x80020002,
        SCE_GXM_ERROR_OUT_OF_MEMORY = 0x80020003,
        SCE_GXM_ERROR_BUSY = 0x80020004,
        SCE_GXM_ERROR_INVALID_VALUE = 0x80020005,
        SCE_GXM_ERROR_BAD_PROGRAM = 0x80020006,
        SCE_GXM_ERROR_UNINITIALIZED = 0x80020007,
        SCE_GXM_ERROR_ALREADY_INITIALIZED = 0x80020008,
        SCE_GXM_ERROR_INVALID_STATE = 0x80020009,
        SCE_GXM_ERROR_INVALID_ARGUMENT = 0x8002000A,
        SCE_GXM_ERROR_DRIVER_INTERNAL = 0x8002000B,
        SCE_GXM_ERROR_INVALID_INDEX_COUNT = 0x8002000C
    };
}

// 1. Macros at the very top
#define SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE (16 * 1024 * 1024)
#define SCE_GXM_DEFAULT_COMMAND_BUFFER_SIZE (4 * 1024 * 1024)
#define SCE_GXM_SHADER_MAGIC 0x474D5853
#define SCE_GXM_SHADER_VERSION 0x00010000
#define MIN_SHADER_SIZE 0x40
#define MAX_SHADER_SIZE 0x100000
#define CACHE_CLEANUP_INTERVAL 60
#define CACHE_TIMEOUT 300
#define MIN_CACHE_HITS 2
#define MAX_SHADER_PARAMETERS 16
#define MAX_SHADER_ATTRIBUTES 16
#define MAX_SHADER_VARYINGS 16
#define MIN_SHADER_CODE_SIZE 0x10
#define MAX_SHADER_CODE_SIZE 0x100000
#define SCE_GXM_MAX_TEXTURE_UNITS 16
#define SCE_GXM_MAX_VERTEX_STREAMS 16
#define SCE_GXM_MAX_VERTEX_ATTRIBUTES 16
#define SCE_GXM_MAX_FRAGMENT_TEXTURES 16
#define SCE_GXM_MAX_UNIFORM_BUFFERS 14

// Move all full struct definitions here, before any function/class that uses them
// SceGxmInitParams
struct SceGxmInitParams {
    uint32_t maxPoolSize;
    // Add other fields as needed
};

// ShaderTranslationState
struct ShaderTranslationState {
    std::string glslCode;
    std::unordered_map<std::string, int> uniformBindings;
    std::unordered_map<std::string, int> attributeBindings;
    std::vector<std::string> uniformNames;
    std::vector<std::string> attributeNames;
    bool translationSuccessful;
    std::string errorMessage;
};

// SceGxmProgram
struct SceGxmProgram {
    uint32_t magic;
    uint32_t version;
    uint32_t headerSize;
    uint32_t programSize;
    uint32_t type;
    uint32_t validationFlags;
    uint32_t validationErrors;
    std::string lastValidationError;
    time_t lastValidation;
    uint64_t memoryUsage;
    uint64_t peakMemoryUsage;
    uint64_t allocationCount;
    uint64_t deallocationCount;
    uint64_t allocationFailures;
    uint64_t compileTime;
    uint64_t linkTime;
    uint32_t compileAttempts;
    uint32_t linkAttempts;
    uint32_t cacheHits;
    uint32_t cacheMisses;
    bool isCached;
    uint32_t parameterCount;
    uint32_t attributeCount;
    uint32_t uniformCount;
    void* data;
    uint32_t size;
};

// ShaderCacheEntry
struct ShaderCacheEntry {
    GLuint program;
    uint32_t cacheHits;
    uint32_t cacheMisses;
    uint64_t memoryUsage;
    uint64_t lastAccess;
};

// ShaderCache
struct ShaderCache {
    std::mutex cacheMutex;
    time_t lastCleanup;
    std::unordered_map<unsigned int, ShaderCacheEntry> entries;
};

// SceGxmContextInitParams
struct SceGxmContextInitParams {
    uint32_t hostMemSize;
    uint32_t parameterBufferSize;
    uint32_t commandBufferSize;
    uint32_t vdmRingBufferSize;
    uint32_t vertexRingBufferSize;
    uint32_t fragmentRingBufferSize;
    uint32_t fragmentUsseRingBufferSize;
    uint32_t vertexUsseRingBufferSize;
    uint32_t driverMemBlockSize;
};

// SceGxmContextStats
struct SceGxmContextStats {
    uint64_t drawCalls;
    uint64_t framesRendered;
    uint64_t memoryUsage;
    uint64_t peakMemoryUsage;
    uint64_t totalAllocations;
    uint64_t totalDeallocations;
    uint64_t allocationFailures;
    uint64_t memoryLeakCount;
    double averageFrameTime;
    uint32_t maxConcurrentDraws;
};

// SceGxmShaderProfile
struct SceGxmShaderProfile {
    uint64_t totalTime;
    uint64_t drawCalls;
    uint64_t shaderSwitches;
    uint64_t cacheHits;
    uint64_t cacheMisses;
    uint64_t memoryUsage;
    uint64_t peakMemory;
    uint32_t maxConcurrentPrograms;
};

// SceGxmMemoryStats
struct SceGxmMemoryStats {
    uint64_t totalAllocated;
    uint64_t totalDeallocated;
    uint64_t currentAllocated;
    uint64_t peakAllocated;
    uint64_t allocationCount;
    uint64_t deallocationCount;
    uint64_t allocationFailures;
    uint64_t peakAllocationCount;
};

// Forward declarations for types used as pointers or members before their use
struct SceGxmContext;
enum class SceGxmContextState : int;

// ContextPool
struct ContextPool {
    std::vector<SceGxmContext*> idleContexts;
    std::mutex poolMutex;
    std::condition_variable poolCondition;
    uint32_t maxPoolSize = 0;
    uint32_t currentPoolSize = 0;
    uint32_t activeContexts = 0;
    uint32_t poolHits = 0;
    uint32_t poolMisses = 0;
    time_t lastCleanup = 0;
};

// MemoryTracking
struct MemoryTracking {
    uint64_t totalAllocated = 0;
    uint64_t totalDeallocated = 0;
    uint64_t currentAllocated = 0;
    uint64_t peakAllocated = 0;
    uint64_t allocationCount = 0;
    uint64_t deallocationCount = 0;
    uint64_t allocationFailures = 0;
    uint64_t peakAllocationCount = 0;
    std::unordered_map<void*, size_t> allocations;
};

// SceGxmContext
typedef struct SceGxmContext {
    // Memory buffers
    void* hostMem;
    uint32_t hostMemSize;
    void* parameterBuffer;
    uint32_t parameterBufferSize;
    void* commandBuffer;
    uint32_t commandBufferSize;
    void* vdmRingBuffer;
    uint32_t vdmRingBufferSize;
    void* vertexRingBuffer;
    uint32_t vertexRingBufferSize;
    void* fragmentRingBuffer;
    uint32_t fragmentRingBufferSize;
    void* fragmentUsseRingBuffer;
    uint32_t fragmentUsseRingBufferSize;
    void* vertexUsseRingBuffer;
    uint32_t vertexUsseRingBufferSize;
    void* driverMemBlock;
    uint32_t driverMemBlockSize;
    
    // State management
    SceGxmContextState state;
    std::mutex stateMutex;
    std::condition_variable stateCondition;
    
    // Synchronization
    std::mutex drawMutex;
    std::condition_variable drawCondition;
    bool isDrawing;
    
    // Context tracking
    uint32_t contextId;
    time_t lastUsed;
    
    // Statistics
    SceGxmContextStats stats;
    
    // Memory tracking
    SceGxmMemoryStats memoryStats;
    
    // Validation
    uint32_t validationFlags;
    uint32_t validationErrors;
    std::string lastValidationError;
    
    // Pooling
    bool isPooled;
    time_t lastPoolAccess;
    uint32_t poolRefCount;
} SceGxmContext;

// SceGxmRenderTarget
typedef struct SceGxmRenderTarget {
    uint32_t width;
    uint32_t height;
    uint32_t strideInPixels;
    uint32_t multisampleMode;
    uint32_t scenesPerFrame;
    void* driverMemBlock;
    uint32_t driverMemBlockSize;
} SceGxmRenderTarget;

// SceGxmColorSurface
typedef struct SceGxmColorSurface {
    uint32_t colorFormat;
    uint32_t surfaceType;
    uint32_t strideInPixels;
    void* data;
    void* colorTarget;
} SceGxmColorSurface;

// SceGxmDepthStencilSurface
typedef struct SceGxmDepthStencilSurface {
    uint32_t depthFormat;
    uint32_t stencilFormat;
    uint32_t surfaceType;
    uint32_t strideInPixels;
    void* depthData;
    void* stencilData;
    void* depthStencilTarget;
} SceGxmDepthStencilSurface;

// SceGxmShaderPatcher
struct SceGxmShaderPatcher {
    void* data;
    uint32_t dataSize;
    uint32_t nextProgramId;
    ShaderCache shaderCache;
    std::mutex cacheMutex;
    time_t lastCleanup;
};

// SceGxmVertexProgram
typedef struct SceGxmVertexProgram {
    const SceGxmProgram* programId;
    void* programBuffer;
    uint32_t programSize;
} SceGxmVertexProgram;

// SceGxmFragmentProgram
typedef struct SceGxmFragmentProgram {
    const SceGxmProgram* programId;
    void* programBuffer;
    uint32_t programSize;
} SceGxmFragmentProgram;

// SceGxmTexture
typedef struct SceGxmTexture {
    uint32_t controlWords[4];
} SceGxmTexture;

// Move all function declarations here, after struct definitions and before any class declarations
int sceGxmInitialize(const void* params);
int sceGxmCreateContext(const void* params, SceGxmContext** context);
int sceGxmDestroyContext(SceGxmContext* context);
int sceGxmSetContext(SceGxmContext* context);
int sceGxmSetVertexProgram(SceGxmContext* context, const SceGxmProgram* program);
int sceGxmSetFragmentProgram(SceGxmContext* context, const SceGxmProgram* program);
int sceGxmDraw(SceGxmContext* context, uint32_t primitiveType, uint32_t indexCount, uint32_t indexOffset);
int sceGxmSetClearColor(SceGxmContext* context, uint32_t color);
int sceGxmCreateRenderTarget(const void* params, SceGxmRenderTarget** renderTarget);
int sceGxmDestroyRenderTarget(SceGxmRenderTarget* renderTarget);
int sceGxmColorSurfaceInit(SceGxmColorSurface* surface, int format, int width, int height, int stride, uint32_t base, uint32_t pitch, uint32_t slice, void* data);
int sceGxmDepthStencilSurfaceInit(SceGxmDepthStencilSurface* surface, int format, int width, int height, uint32_t base, void* data, void* stencilData);
static std::string translateShader(const void* vitaShader, size_t size, bool isVertex);
bool validateShaderFormat(const void* data, uint32_t size);

#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <SDL2/SDL.h>

// Move all class declarations here, after function declarations
class GxmMemoryManager {
public:
    void* allocate(size_t size);
    void deallocate(void* ptr);
    void* allocateAligned(size_t size, size_t alignment);
    void* allocateHostMemory(size_t size);
    void* allocateParameterBuffer(size_t size);
    void* allocateCommandBuffer(size_t size);
    void* allocateRingBuffer(size_t size);
    void* allocateUsseBuffer(size_t size);
    void* allocateDriverMemory(size_t size);
    size_t getCurrentAllocated() const { return total_allocated; }
    
private:
    struct MemoryBlock {
        void* ptr;
        size_t size;
        bool is_aligned;
    };
    
    std::unordered_map<void*, MemoryBlock> allocated_memory;
    std::vector<MemoryBlock> free_blocks;
    size_t total_allocated = 0;
    size_t max_allocation = SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE * 2;
};

class SwitchRenderer;

// GXM constants
#define SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE (16 * 1024 * 1024)
#define SCE_GXM_DEFAULT_COMMAND_BUFFER_SIZE (4 * 1024 * 1024)
#define SCE_GXM_SHADER_MAGIC 0x474D5853
#define SCE_GXM_SHADER_VERSION 0x00010000
#define MIN_SHADER_SIZE 0x40
#define MAX_SHADER_SIZE 0x100000
#define CACHE_CLEANUP_INTERVAL 60
#define CACHE_TIMEOUT 300
#define MIN_CACHE_HITS 2
#define MAX_SHADER_PARAMETERS 16
#define MAX_SHADER_ATTRIBUTES 16
#define MAX_SHADER_VARYINGS 16
#define MIN_SHADER_CODE_SIZE 0x10
#define MAX_SHADER_CODE_SIZE 0x100000
#define SCE_GXM_MAX_TEXTURE_UNITS 16
#define SCE_GXM_MAX_VERTEX_STREAMS 16
#define SCE_GXM_MAX_VERTEX_ATTRIBUTES 16
#define SCE_GXM_MAX_FRAGMENT_TEXTURES 16
#define SCE_GXM_MAX_UNIFORM_BUFFERS 14

// Forward declarations for all types used in function signatures
struct SceGxmProgram;
struct SceGxmContext;
struct SceGxmRenderTarget;
struct SceGxmColorSurface;
struct SceGxmDepthStencilSurface;
struct SceGxmVertexProgram;
struct SceGxmFragmentProgram;
struct SceGxmTexture;
struct SceGxmShaderPatcher;

typedef int SceGxmVertexProgramInputFormat; // TODO: Define properly if needed
typedef int SceGxmVertexStreamMask;         // TODO: Define properly if needed
typedef int SceGxmOutputRegisterFormat;     // TODO: Define properly if needed
typedef int SceGxmMultisampleMode;          // TODO: Define properly if needed
typedef int SceGxmIndexFormat;              // TODO: Define properly if needed

bool validateShaderFormat(const void* data, uint32_t size);

// GXM primitive types
typedef enum SceGxmPrimitiveType {
    SCE_GXM_PRIMITIVE_TRIANGLES = 0,
    SCE_GXM_PRIMITIVE_TRIANGLE_STRIP = 1,
    SCE_GXM_PRIMITIVE_TRIANGLE_FAN = 2,
    SCE_GXM_PRIMITIVE_LINES = 3,
    SCE_GXM_PRIMITIVE_LINE_STRIP = 4,
    SCE_GXM_PRIMITIVE_POINTS = 5
} SceGxmPrimitiveType;

// GXM texture formats
typedef enum SceGxmTextureFormat {
    SCE_GXM_TEXTURE_FORMAT_U8 = 0x00000000,
    SCE_GXM_TEXTURE_FORMAT_U4U4U4U4 = 0x00000001,
    SCE_GXM_TEXTURE_FORMAT_U8U8U8U8 = 0x00000002,
    SCE_GXM_TEXTURE_FORMAT_U8U8U8 = 0x00000003,
    SCE_GXM_TEXTURE_FORMAT_U5U6U5 = 0x00000004,
    SCE_GXM_TEXTURE_FORMAT_U1U5U5U5 = 0x00000005,
    SCE_GXM_TEXTURE_FORMAT_U8_R111 = 0x00000006,
    SCE_GXM_TEXTURE_FORMAT_U8_111R = 0x00000007,
    SCE_GXM_TEXTURE_FORMAT_U8_1RRR = 0x00000008,
    SCE_GXM_TEXTURE_FORMAT_U8_R1RR = 0x00000009,
    SCE_GXM_TEXTURE_FORMAT_U8_RR1R = 0x0000000A,
    SCE_GXM_TEXTURE_FORMAT_U8_RRR1 = 0x0000000B,
    SCE_GXM_TEXTURE_FORMAT_UBC1 = 0x0000000C,
    SCE_GXM_TEXTURE_FORMAT_UBC2 = 0x0000000D,
    SCE_GXM_TEXTURE_FORMAT_UBC3 = 0x0000000E,
    SCE_GXM_TEXTURE_FORMAT_UBC4 = 0x0000000F,
    SCE_GXM_TEXTURE_FORMAT_UBC5 = 0x00000010,
    SCE_GXM_TEXTURE_FORMAT_PVRT2BPP = 0x00000011,
    SCE_GXM_TEXTURE_FORMAT_PVRT4BPP = 0x00000012,
    SCE_GXM_TEXTURE_FORMAT_PVRTII2BPP = 0x00000013,
    SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP = 0x00000014,
    SCE_GXM_TEXTURE_FORMAT_P4 = 0x00000015,
    SCE_GXM_TEXTURE_FORMAT_P8 = 0x00000016,
    SCE_GXM_TEXTURE_FORMAT_U8U8 = 0x00000017,
    SCE_GXM_TEXTURE_FORMAT_S8S8 = 0x00000018,
    SCE_GXM_TEXTURE_FORMAT_S8 = 0x00000019,
    SCE_GXM_TEXTURE_FORMAT_U16 = 0x0000001A,
    SCE_GXM_TEXTURE_FORMAT_S16 = 0x0000001B,
    SCE_GXM_TEXTURE_FORMAT_F16 = 0x0000001C,
    SCE_GXM_TEXTURE_FORMAT_U16U16 = 0x0000001D,
    SCE_GXM_TEXTURE_FORMAT_S16S16 = 0x0000001E,
    SCE_GXM_TEXTURE_FORMAT_F16F16 = 0x0000001F,
    SCE_GXM_TEXTURE_FORMAT_F32 = 0x00000020,
    SCE_GXM_TEXTURE_FORMAT_F32M = 0x00000021,
    SCE_GXM_TEXTURE_FORMAT_X8S8S8U8 = 0x00000022,
    SCE_GXM_TEXTURE_FORMAT_X8U8S8S8 = 0x00000023,
    SCE_GXM_TEXTURE_FORMAT_U32 = 0x00000024,
    SCE_GXM_TEXTURE_FORMAT_S32 = 0x00000025,
    SCE_GXM_TEXTURE_FORMAT_SE5M9M9M9 = 0x00000026,
    SCE_GXM_TEXTURE_FORMAT_F11F11F10 = 0x00000027,
    SCE_GXM_TEXTURE_FORMAT_F16F16F16F16 = 0x00000028,
    SCE_GXM_TEXTURE_FORMAT_U16U16U16U16 = 0x00000029,
    SCE_GXM_TEXTURE_FORMAT_S16S16S16S16 = 0x0000002A,
    SCE_GXM_TEXTURE_FORMAT_F32F32 = 0x0000002B,
    SCE_GXM_TEXTURE_FORMAT_U32U32 = 0x0000002C,
    SCE_GXM_TEXTURE_FORMAT_F32F32F32F32 = 0x0000002D,
    SCE_GXM_TEXTURE_FORMAT_U32U32U32U32 = 0x0000002E,
    SCE_GXM_TEXTURE_FORMAT_YUYV422 = 0x0000002F,
    SCE_GXM_TEXTURE_FORMAT_YUV420P2 = 0x00000030,
    SCE_GXM_TEXTURE_FORMAT_YUV420P3 = 0x00000031,
    SCE_GXM_TEXTURE_FORMAT_YUV422P2 = 0x00000032,
    SCE_GXM_TEXTURE_FORMAT_YUV422P3 = 0x00000033,
    SCE_GXM_TEXTURE_FORMAT_UYVY422 = 0x00000034,
    SCE_GXM_TEXTURE_FORMAT_P4_ABGR = 0x00000035,
    SCE_GXM_TEXTURE_FORMAT_P4_ARGB = 0x00000036,
    SCE_GXM_TEXTURE_FORMAT_P4_RGBA = 0x00000037,
    SCE_GXM_TEXTURE_FORMAT_P4_BGRA = 0x00000038,
    SCE_GXM_TEXTURE_FORMAT_P4_RGB = 0x00000039,
    SCE_GXM_TEXTURE_FORMAT_P4_BGR = 0x0000003A,
    SCE_GXM_TEXTURE_FORMAT_P8_ABGR = 0x0000003B,
    SCE_GXM_TEXTURE_FORMAT_P8_ARGB = 0x0000003C,
    SCE_GXM_TEXTURE_FORMAT_P8_RGBA = 0x0000003D,
    SCE_GXM_TEXTURE_FORMAT_P8_BGRA = 0x0000003E,
    SCE_GXM_TEXTURE_FORMAT_P8_RGB = 0x0000003F,
    SCE_GXM_TEXTURE_FORMAT_P8_BGR = 0x00000040
} SceGxmTextureFormat;

// GXM shader program types
typedef enum SceGxmProgramType {
    SCE_GXM_PROGRAM_TYPE_VERTEX = 0,
    SCE_GXM_PROGRAM_TYPE_FRAGMENT = 1
} SceGxmProgramType;


// GXM context state
enum class SceGxmContextState {
    UNINITIALIZED,
    INITIALIZED,
    READY,
    DRAWING,
    FINISHED,
    DESTROYED
};

// Type definitions for GXM enums that were forward-referenced
enum class SceGxmShaderValidationFlags : uint32_t {
    VALIDATION_NONE = 0,
    VALIDATION_SHADER_SIZE = 1 << 0,
    VALIDATION_SHADER_TYPE = 1 << 1,
    VALIDATION_SHADER_FORMAT = 1 << 2,
    VALIDATION_SHADER_VALID = 1 << 3
};
enum class SceGxmShaderValidationError : uint32_t {
    ERROR_NONE = 0,
    ERROR_INVALID_SIZE = 1 << 0,
    ERROR_INVALID_TYPE = 1 << 1,
    ERROR_INVALID_FORMAT = 1 << 2,
    ERROR_MEMORY_LEAK = 1 << 3
};

// SceGxm module implementation
class SceGxm : public Module {
public:
    SceGxm(const std::string& name);
    ~SceGxm();

    // Context pool management
    void initializeContextPool(uint32_t maxPoolSize);
    SceGxmContext* acquireContext(const SceGxmContextInitParams& params);
    void releaseContext(SceGxmContext* context);
    void cleanupContextPool();
    void dumpPoolStats();
    void dumpMemoryStats();
    
    // Initialize and finalize
    bool initialize(SwitchRenderer* renderer);
    void finalize();
    
    // Context management
    int sceGxmInitialize(const void* params);
    int sceGxmTerminate();
    int sceGxmCreateContext(const void* params, SceGxmContext** context);
    int sceGxmDestroyContext(SceGxmContext* context);
    int sceGxmSetClearColor(SceGxmContext* context, uint32_t color);
    int sceGxmSetContext(SceGxmContext* context);
    bool validateContext(SceGxmContext* context);
    static int sceGxmTerminateWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmCreateContextWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmDestroyContextWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmSetClearColorWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    
    // Render target management
    int sceGxmCreateRenderTarget(const void* params, SceGxmRenderTarget** renderTarget);
    int sceGxmDestroyRenderTarget(SceGxmRenderTarget* renderTarget);
    static int sceGxmCreateRenderTargetWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmDestroyRenderTargetWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    
    // Surface management
    int sceGxmColorSurfaceInit(SceGxmColorSurface* surface, int colorFormat, int surfaceType, int scaleMode, int outputRegisterSize, uint32_t width, uint32_t height, uint32_t strideInPixels, void* data);
    int sceGxmDepthStencilSurfaceInit(SceGxmDepthStencilSurface* surface, int depthStencilFormat, int surfaceType, uint32_t strideInPixels, void* depthData, void* stencilData);
    static int sceGxmColorSurfaceInitWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmDepthStencilSurfaceInitWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    
    // Shader management
    int sceGxmShaderPatcherCreate(const void* params, SceGxmShaderPatcher** shaderPatcher);
    int sceGxmShaderPatcherDestroy(SceGxmShaderPatcher* shaderPatcher);
    int sceGxmShaderPatcherRegisterProgram(SceGxmShaderPatcher* shaderPatcher, const SceGxmProgram* program, uint32_t* programId);
    int sceGxmShaderPatcherUnregisterProgram(SceGxmShaderPatcher* shaderPatcher, const SceGxmProgram* program);
    int sceGxmShaderPatcherCreateVertexProgram(SceGxmShaderPatcher* shaderPatcher, uint32_t programId, int inputFormat, int vertexStreamMask, const void* vertexCallback, void* vertexCallbackData, SceGxmVertexProgram** vertexProgram);
    int sceGxmShaderPatcherCreateFragmentProgram(SceGxmShaderPatcher* shaderPatcher, uint32_t programId, int outputFormat, int multisampleMode, const void* blendInfo, const SceGxmProgram* vertexProgram, SceGxmFragmentProgram** fragmentProgram);
    static int sceGxmShaderPatcherCreateWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmShaderPatcherDestroyWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmShaderPatcherRegisterProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmShaderPatcherUnregisterProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmShaderPatcherCreateVertexProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmShaderPatcherCreateFragmentProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    
    // Texture management
    int sceGxmTextureInitLinear(SceGxmTexture* texture, const void* data, SceGxmTextureFormat texFormat, uint32_t width, uint32_t height, uint32_t mipCount);
    int sceGxmTextureSetData(SceGxmTexture* texture, const void* data);
    int sceGxmTextureSetFormat(SceGxmTexture* texture, SceGxmTextureFormat texFormat);
    int sceGxmTextureSetWidth(SceGxmTexture* texture, uint32_t width);
    int sceGxmTextureSetHeight(SceGxmTexture* texture, uint32_t height);
    static int sceGxmTextureInitLinearWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmTextureSetDataWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmTextureSetFormatWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmTextureSetWidthWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmTextureSetHeightWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    
    // Drawing
    int sceGxmBeginScene(SceGxmContext* context, uint32_t flags, const SceGxmRenderTarget* renderTarget, const void* displayQueueCallback, void* displayQueueCallbackData, const SceGxmColorSurface* colorSurface, const SceGxmDepthStencilSurface* depthStencilSurface);
    int sceGxmEndScene(SceGxmContext* context, const void* vertexCallback, void* vertexCallbackData, const void* fragmentCallback, void* fragmentCallbackData);
    int sceGxmSetVertexProgram(SceGxmContext* context, const SceGxmVertexProgram* vertexProgram);
    int sceGxmSetFragmentProgram(SceGxmContext* context, const SceGxmFragmentProgram* fragmentProgram);
    // Draw APIs
    int sceGxmDraw(SceGxmContext* context,
                   SceGxmPrimitiveType primType,
                   SceGxmIndexFormat indexType,
                   const void* indexData,
                   uint32_t indexCount);

    // Overload matching implementation and wrappers (vertex data focused)
    int sceGxmDraw(SceGxmContext* context,
                   SceGxmPrimitiveType primType,
                   int vertexCount,
                   const void* vertexData,
                   uint32_t vertexSize,
                   const void* indexData = nullptr,
                   uint32_t indexCount = 0,
                   uint32_t indexType = 0);
    int sceGxmDrawInstanced(SceGxmContext* context, SceGxmPrimitiveType primType, SceGxmIndexFormat indexType, const void* indexData, uint32_t indexCount, uint32_t instanceCount);
    static int sceGxmBeginSceneWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmEndSceneWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmSetVertexProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmSetFragmentProgramWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmDrawWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmDrawInstancedWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    static int sceGxmInitializeWrapper(Emulator& emulator, unsigned int nid, const std::vector<unsigned int>& args);
    
    // Shader utilities
    static std::string translateShader(const void* vitaShader, size_t size, bool isVertex);
    static bool validateShaderProgram(SceGxmProgram* program);
    void cleanupShaderCache();
    void updateShaderProfile(SceGxmProgram* program, bool isCacheHit);
    void dumpShaderProfile();
    void registerSyscalls();

    // Member variables
    ContextPool contextPool;
    MemoryTracking memoryTracking;
    SceGxmShaderProfile shaderProfile;
    std::unique_ptr<GxmMemoryManager> memoryManager;
    std::unordered_map<SceGxmContext*, SceGxmContext*> contexts;
    SceGxmContext* currentContext = nullptr;
    uint32_t nextContextId = 1;
    ShaderCache shaderCache; // TODO: Implement full struct definition if not already present

private:
    SwitchRenderer* renderer;
    bool initialized;
    SceGxmRenderTarget* currentRenderTarget;
    SceGxmColorSurface* currentColorSurface;
    SceGxmDepthStencilSurface* currentDepthStencilSurface;
    SceGxmVertexProgram* currentVertexProgram;
    SceGxmFragmentProgram* currentFragmentProgram;
    std::mutex mutex;
    GLuint convertTextureFormat(SceGxmTextureFormat format);
    GLenum convertPrimitiveType(SceGxmPrimitiveType primType);

    // Command emission helpers (implemented in SceGxm.cpp)
    void emitUploadVertexBuffer(const void* data, size_t size);
    void emitUploadIndexBuffer(const void* data, size_t size, uint32_t indexType);
    void emitUploadShader(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
    void emitSetupVertexAttributes(const std::vector<std::tuple<int, int, GLenum, size_t>>& layout, size_t stride);
    void emitSetUniform(const std::string& name, const std::vector<float>& values);
    void emitSetSampler(const std::string& name, int unit);
};
