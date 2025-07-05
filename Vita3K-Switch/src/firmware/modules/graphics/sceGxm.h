#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <string>

typedef uint32_t SceGxmShaderPatcherId;

namespace firmware {
namespace modules {
namespace graphics {

// Forward declarations within graphics namespace
struct SceGxmNotification;
struct SceGxmProgramParameter;
struct SceGxmProgram;

// GXM error codes
enum SceGxmErrorCode {
    SCE_GXM_ERROR_INVALID_POINTER       = 0x805B0000,
    SCE_GXM_ERROR_INVALID_VALUE         = 0x805B0001,
    SCE_GXM_ERROR_INVALID_ENUM          = 0x805B0002,
    SCE_GXM_ERROR_INVALID_ALIGNMENT     = 0x805B0003,
    SCE_GXM_ERROR_INVALID_SIZE          = 0x805B0004,
    SCE_GXM_ERROR_INVALID_ADDRESS       = 0x805B0005,
    SCE_GXM_ERROR_INVALID_INDEX_COUNT   = 0x805B0006,
    SCE_GXM_ERROR_INVALID_POLYGON_MODE  = 0x805B0007,
    SCE_GXM_ERROR_UNINITIALIZED         = 0x805B000A,
    SCE_GXM_ERROR_ALREADY_INITIALIZED   = 0x805B000B,
    SCE_GXM_ERROR_FEATURE_UNSUPPORTED   = 0x805B000C,
    SCE_GXM_ERROR_OUT_OF_MEMORY         = 0x805B000D,
    SCE_GXM_ERROR_OUT_OF_RENDERTARGETS  = 0x805B0010,
    SCE_GXM_ERROR_SHADER_ALREADY_COMPILED = 0x805B0011,
    SCE_GXM_ERROR_THREAD_SHUTDOWN       = 0x805B0012,
    SCE_GXM_ERROR_PATCHER_INTERNAL      = 0x805B0013,
    SCE_GXM_ERROR_TILED_MEMORY_TOO_SMALL = 0x805B0014,
    SCE_GXM_ERROR_DRIVER_INTERNAL       = 0x805B0015
};

// GXM constants
#define SCE_GXM_DEFAULT_PARAMETER_BUFFER_SIZE 16 * 1024 * 1024
#define SCE_GXM_DEFAULT_COMMAND_BUFFER_SIZE 2 * 1024 * 1024
#define SCE_GXM_MINIMUM_COMMAND_BUFFER_SIZE 32 * 1024
#define SCE_GXM_MINIMUM_PARAMETER_BUFFER_SIZE 32 * 1024

// GXM memory layout
#define SCE_GXM_MEMORY_ATTRIB_READ      0x1
#define SCE_GXM_MEMORY_ATTRIB_WRITE     0x2
#define SCE_GXM_MEMORY_ATTRIB_RW        (SCE_GXM_MEMORY_ATTRIB_READ | SCE_GXM_MEMORY_ATTRIB_WRITE)

// GXM primitive types
#define SCE_GXM_PRIMITIVE_TRIANGLES           0x00000000U
#define SCE_GXM_PRIMITIVE_TRIANGLE_STRIP      0x00000001U
#define SCE_GXM_PRIMITIVE_TRIANGLE_FAN        0x00000002U
#define SCE_GXM_PRIMITIVE_LINES               0x00000003U
#define SCE_GXM_PRIMITIVE_LINE_STRIP          0x00000004U
#define SCE_GXM_PRIMITIVE_POINTS              0x00000005U

// GXM texture formats
#define SCE_GXM_TEXTURE_FORMAT_U8                    0x00000000U
#define SCE_GXM_TEXTURE_FORMAT_U4U4U4U4              0x00000001U
#define SCE_GXM_TEXTURE_FORMAT_U8U8                  0x00000002U
#define SCE_GXM_TEXTURE_FORMAT_U5U6U5                0x00000003U
#define SCE_GXM_TEXTURE_FORMAT_U1U5U5U5              0x00000004U
#define SCE_GXM_TEXTURE_FORMAT_U8U8U8                0x00000005U
#define SCE_GXM_TEXTURE_FORMAT_U8U8U8U8              0x00000006U
#define SCE_GXM_TEXTURE_FORMAT_F16                   0x0000000AU
#define SCE_GXM_TEXTURE_FORMAT_F16F16                0x0000000BU
#define SCE_GXM_TEXTURE_FORMAT_F32                   0x0000000CU
#define SCE_GXM_TEXTURE_FORMAT_F32F32                0x0000000DU
#define SCE_GXM_TEXTURE_FORMAT_F32M                  0x00000010U
#define SCE_GXM_TEXTURE_FORMAT_X8S8S8U8              0x00000012U
#define SCE_GXM_TEXTURE_FORMAT_X8U24                 0x00000013U
#define SCE_GXM_TEXTURE_FORMAT_U32                   0x00000014U
#define SCE_GXM_TEXTURE_FORMAT_F16F16F16F16          0x00000017U
#define SCE_GXM_TEXTURE_FORMAT_F32F32F32F32          0x00000018U
#define SCE_GXM_TEXTURE_FORMAT_PVRT2BPP              0x80000000U
#define SCE_GXM_TEXTURE_FORMAT_PVRT4BPP              0x80000001U
#define SCE_GXM_TEXTURE_FORMAT_PVRTII2BPP            0x80000002U
#define SCE_GXM_TEXTURE_FORMAT_PVRTII4BPP            0x80000003U

// GXM texture filter modes
#define SCE_GXM_TEXTURE_FILTER_POINT              0x00000000U
#define SCE_GXM_TEXTURE_FILTER_LINEAR             0x00000001U

// GXM texture address modes
#define SCE_GXM_TEXTURE_ADDR_REPEAT               0x00000000U
#define SCE_GXM_TEXTURE_ADDR_MIRROR               0x00000001U
#define SCE_GXM_TEXTURE_ADDR_CLAMP                0x00000002U
#define SCE_GXM_TEXTURE_ADDR_BORDER               0x00000003U
#define SCE_GXM_TEXTURE_ADDR_MIRROR_CLAMP         0x00000004U

// GXM blend modes
#define SCE_GXM_BLEND_FUNC_NONE                   0x00000000U
#define SCE_GXM_BLEND_FUNC_ADD                    0x00000001U
#define SCE_GXM_BLEND_FUNC_SUBTRACT               0x00000002U
#define SCE_GXM_BLEND_FUNC_REVERSE_SUBTRACT       0x00000003U
#define SCE_GXM_BLEND_FUNC_MIN                    0x00000004U
#define SCE_GXM_BLEND_FUNC_MAX                    0x00000005U

// GXM blend factors
#define SCE_GXM_BLEND_FACTOR_ZERO                 0x00000000U
#define SCE_GXM_BLEND_FACTOR_ONE                  0x00000001U
#define SCE_GXM_BLEND_FACTOR_SRC_COLOR            0x00000002U
#define SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_COLOR  0x00000003U
#define SCE_GXM_BLEND_FACTOR_DST_COLOR            0x00000004U
#define SCE_GXM_BLEND_FACTOR_ONE_MINUS_DST_COLOR  0x00000005U
#define SCE_GXM_BLEND_FACTOR_SRC_ALPHA            0x00000006U
#define SCE_GXM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA  0x00000007U
#define SCE_GXM_BLEND_FACTOR_DST_ALPHA            0x00000008U
#define SCE_GXM_BLEND_FACTOR_ONE_MINUS_DST_ALPHA  0x00000009U
#define SCE_GXM_BLEND_FACTOR_SRC_ALPHA_SATURATE   0x0000000AU
#define SCE_GXM_BLEND_FACTOR_DST_ALPHA_SATURATE   0x0000000BU

// GXM depth/stencil test functions
#define SCE_GXM_DEPTH_FUNC_NEVER                  0x00000000U
#define SCE_GXM_DEPTH_FUNC_LESS                   0x00000001U
#define SCE_GXM_DEPTH_FUNC_EQUAL                  0x00000002U
#define SCE_GXM_DEPTH_FUNC_LESS_EQUAL             0x00000003U
#define SCE_GXM_DEPTH_FUNC_GREATER                0x00000004U
#define SCE_GXM_DEPTH_FUNC_NOT_EQUAL              0x00000005U
#define SCE_GXM_DEPTH_FUNC_GREATER_EQUAL          0x00000006U
#define SCE_GXM_DEPTH_FUNC_ALWAYS                 0x00000007U

// Forward declarations
struct SceGxmContext {
    // Minimal definition to unblock build
    // Add actual fields as needed for emulation
};
struct SceGxmRenderTarget {
    // Minimal definition to unblock build
    // Add actual fields as needed for emulation
};
struct SceGxmVertexProgram {
    // Minimal definition to unblock build
    // Add actual fields as needed for emulation
};
struct SceGxmFragmentProgram {
    // Minimal definition to unblock build
    // Add actual fields as needed for emulation
};
struct SceGxmShaderPatcher {
    // Minimal definition to unblock build
    // Add actual fields as needed for emulation
};
struct SceGxmSyncObject;
struct SceGxmTexture;
struct SceGxmColorSurface;
struct SceGxmDepthStencilSurface;

// GXM initialization parameters
struct SceGxmInitializeParams {
    uint32_t flags;
    uint32_t displayQueueMaxPendingCount;
    void *displayQueueCallback;
    uint32_t displayQueueCallbackDataSize;
    void *parameterBufferSize;
};

// GXM context parameters
struct SceGxmContextParams {
    void *hostMem;
    uint32_t hostMemSize;
    void *vdmRingBufferMem;
    uint32_t vdmRingBufferMemSize;
    void *vertexRingBufferMem;
    uint32_t vertexRingBufferMemSize;
    void *fragmentRingBufferMem;
    uint32_t fragmentRingBufferMemSize;
    void *fragmentUsseRingBufferMem;
    uint32_t fragmentUsseRingBufferMemSize;
    uint32_t fragmentUsseRingBufferOffset;
};

// GXM render target parameters
struct SceGxmRenderTargetParams {
    uint32_t flags;
    uint16_t width;
    uint16_t height;
    uint16_t scenesPerFrame;
    uint16_t multisampleMode;
    void *driverMemBlock;
    void *hostMem;
    uint32_t hostMemSize;
};

// GXM texture
struct SceGxmTexture {
    uint32_t controlWords[4];
};

// GXM color surface
struct SceGxmColorSurface {
    uint32_t flags;
    uint32_t format;
    void *data;
    uint32_t strideInPixels;
};

// GXM depth stencil surface
struct SceGxmDepthStencilSurface {
    uint32_t flags;
    uint32_t format;
    void *depthData;
    void *stencilData;
    uint32_t strideInSamples;
};

// GXM vertex attribute
struct SceGxmVertexAttribute {
    uint16_t streamIndex;
    uint16_t offset;
    uint8_t format;
    uint8_t componentCount;
    uint16_t regIndex;
};

// GXM vertex stream
struct SceGxmVertexStream {
    uint16_t streamIndex;
    uint16_t stride;
    void *data;
};

// GXM shader patcher parameters
struct SceGxmShaderPatcherParams {
    void *userData;
    void *hostAllocCallback;
    void *hostFreeCallback;
    void *bufferAllocCallback;
    void *bufferFreeCallback;
    uint32_t bufferMem;
    uint32_t bufferMemSize;
    uint32_t vertexUsseAllocCallback;
    uint32_t vertexUsseFreeCallback;
    uint32_t vertexUsseMem;
    uint32_t vertexUsseMemSize;
    uint32_t vertexUsseOffset;
    uint32_t fragmentUsseAllocCallback;
    uint32_t fragmentUsseFreeCallback;
    uint32_t fragmentUsseMem;
    uint32_t fragmentUsseMemSize;
    uint32_t fragmentUsseOffset;
};

// GXM shader program
struct SceGxmProgram {
    uint32_t magic;
    uint8_t major_version;
    uint8_t minor_version;
    uint16_t padding;
    uint32_t size;
    uint32_t flags;
    uint32_t vertex_shader_reserved;
    uint32_t fragment_shader_reserved;
    uint32_t vertex_program_offset;
    uint32_t fragment_program_offset;
};

// GXM shader binary
struct SceGxmShaderBinary {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t flags;
    uint32_t vertexProgramOffset;
    uint32_t fragmentProgramOffset;
};

// GXM manager class
class SceGxmManager {
public:
    SceGxmManager();
    ~SceGxmManager();

    // Initialize the GXM manager
    int initialize(const SceGxmInitializeParams *params);

    // Terminate the GXM manager
    int terminate();

    // Create a GXM context
    int create_context(SceGxmContext **context, const SceGxmContextParams *params);

    // Destroy a GXM context
    int destroy_context(SceGxmContext *context);

    // Create a render target
    int create_render_target(SceGxmRenderTarget **renderTarget, const SceGxmRenderTargetParams *params);

    // Destroy a render target
    int destroy_render_target(SceGxmRenderTarget *renderTarget);

    // Start a scene
    int start_scene(SceGxmContext *context, uint32_t flags, const SceGxmRenderTarget *renderTarget,
                   const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil);

    // End a scene
    int end_scene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                 const SceGxmNotification *fragmentNotification);

    // Draw primitives
    int draw_primitives(SceGxmContext *context, uint32_t primType, uint32_t indexType,
                       const void *indexData, uint32_t indexCount);

    // Create a shader patcher
    int create_shader_patcher(SceGxmShaderPatcher **shaderPatcher, const SceGxmShaderPatcherParams *params);

    // Destroy a shader patcher
    int destroy_shader_patcher(SceGxmShaderPatcher *shaderPatcher);

    // Create a vertex program
    int create_vertex_program(SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram **vertexProgram,
                             const SceGxmVertexAttribute *attributes, uint32_t attributeCount,
                             const SceGxmVertexStream *streams, uint32_t streamCount,
                             const SceGxmProgram *program);

    // Destroy a vertex program
    int destroy_vertex_program(SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram);

    // Create a fragment program
    int create_fragment_program(SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram **fragmentProgram,
                               const SceGxmProgram *program, uint32_t blendMode,
                               const int *blendInfo, const SceGxmProgram *vertexProgram);

    // Destroy a fragment program
    int destroy_fragment_program(SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram);

    // Set vertex program
    int set_vertex_program(SceGxmContext *context, const SceGxmVertexProgram *vertexProgram);

    // Set fragment program
    int set_fragment_program(SceGxmContext *context, const SceGxmFragmentProgram *fragmentProgram);

    // Set vertex stream
    int set_vertex_stream(SceGxmContext *context, uint32_t streamIndex, const void *data);

    // Set texture
    int set_texture(SceGxmContext *context, uint32_t textureIndex, const SceGxmTexture *texture);

    // Allocate memory for texture
    void* allocate_texture_memory(uint32_t size, uint32_t alignment);

    // Free texture memory
    void free_texture_memory(void *memory);

    // Translate shader
    std::string translate_shader(const SceGxmProgram *program);

    // Helper functions
    bool is_valid_primitive_type(uint32_t primType);
    bool is_valid_index_type(uint32_t indexType);
    bool is_valid_texture_format(uint32_t format);

    // Shader management


private:
    bool initialized;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Initialization parameters
    SceGxmInitializeParams init_params;

    // Contexts
    std::map<SceGxmContext*, std::unique_ptr<SceGxmContext>> contexts;

    // Render targets
    std::map<SceGxmRenderTarget*, std::unique_ptr<SceGxmRenderTarget>> render_targets;

    // Shader patchers
    std::map<SceGxmShaderPatcher*, std::unique_ptr<SceGxmShaderPatcher>> shader_patchers;

    // Vertex programs
    std::map<SceGxmVertexProgram*, std::unique_ptr<SceGxmVertexProgram>> vertex_programs;

    // Fragment programs
    std::map<SceGxmFragmentProgram*, std::unique_ptr<SceGxmFragmentProgram>> fragment_programs;

    // Texture memory
    std::map<void*, uint32_t> texture_memory;

    // Shader cache
    std::map<const SceGxmProgram*, std::string> shader_cache;
};

// GXM notification structure
struct SceGxmNotification {
    void *address;
    uint32_t value;
};

// GXM blend info structure
struct SceGxmBlendInfo {
    uint8_t colorFunc;
    uint8_t alphaFunc;
    uint8_t colorSrc;
    uint8_t colorDst;
    uint8_t alphaSrc;
    uint8_t alphaDst;
    uint8_t colorMask;
    uint8_t padding;
};

// Module functions
int sceGxmInitialize(const SceGxmInitializeParams *params);
int sceGxmTerminate();
int sceGxmCreateContext(SceGxmContext **context, const SceGxmContextParams *params);
int sceGxmDestroyContext(SceGxmContext *context);
int sceGxmCreateRenderTarget(SceGxmRenderTarget **renderTarget, const SceGxmRenderTargetParams *params);
int sceGxmDestroyRenderTarget(SceGxmRenderTarget *renderTarget);
int sceGxmBeginScene(SceGxmContext *context, uint32_t flags, const SceGxmRenderTarget *renderTarget,
                    const SceGxmColorSurface *colorSurface, const SceGxmDepthStencilSurface *depthStencil);
int sceGxmEndScene(SceGxmContext *context, const SceGxmNotification *vertexNotification,
                  const SceGxmNotification *fragmentNotification);
int sceGxmDraw(SceGxmContext *context, uint32_t primType, uint32_t indexType,
              const void *indexData, uint32_t indexCount);
int sceGxmSetVertexProgram(SceGxmContext *context, const SceGxmVertexProgram *vertexProgram);
int sceGxmSetFragmentProgram(SceGxmContext *context, const SceGxmFragmentProgram *fragmentProgram);
int sceGxmSetVertexStream(SceGxmContext *context, uint32_t streamIndex, const void *data);
int sceGxmSetVertexTexture(SceGxmContext *context, uint32_t textureIndex, const SceGxmTexture *texture);
int sceGxmSetFragmentTexture(SceGxmContext *context, uint32_t textureIndex, const SceGxmTexture *texture);
int sceGxmCreateShaderPatcher(SceGxmShaderPatcher **shaderPatcher, const SceGxmShaderPatcherParams *params);
int sceGxmDestroyShaderPatcher(SceGxmShaderPatcher *shaderPatcher);
int sceGxmShaderPatcherCreateVertexProgram(SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram **vertexProgram,
                                         const SceGxmVertexAttribute *attributes, uint32_t attributeCount,
                                         const SceGxmVertexStream *streams, uint32_t streamCount,
                                         const SceGxmProgram *program);
int sceGxmShaderPatcherCreateFragmentProgram(SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram **fragmentProgram,
                                           const SceGxmProgram *program, uint32_t blendMode,
                                           const SceGxmBlendInfo *blendInfo, const SceGxmProgram *vertexProgram);
int sceGxmShaderPatcherDestroy(SceGxmShaderPatcher *shaderPatcher);
int sceGxmShaderPatcherDestroyVertexProgram(SceGxmShaderPatcher *shaderPatcher, SceGxmVertexProgram *vertexProgram);
int sceGxmShaderPatcherDestroyFragmentProgram(SceGxmShaderPatcher *shaderPatcher, SceGxmFragmentProgram *fragmentProgram);
int sceGxmReserveVertexDefaultUniformBuffer(SceGxmContext *context, void **uniformBuffer);
int sceGxmReserveFragmentDefaultUniformBuffer(SceGxmContext *context, void **uniformBuffer);
int sceGxmSetUniformDataF(void *uniformBuffer, const SceGxmProgram *program, uint32_t offset,
                         uint32_t componentOffset, uint32_t componentCount, const float *data);

// Texture functions
int sceGxmTextureInitLinear(SceGxmTexture *texture, const void *data, uint32_t format,
                          uint32_t width, uint32_t height, uint32_t mipCount);
int sceGxmTextureInitSwizzled(SceGxmTexture *texture, const void *data, uint32_t format,
                            uint32_t width, uint32_t height, uint32_t mipCount);
int sceGxmTextureInitCube(SceGxmTexture *texture, const void *data, uint32_t format,
                        uint32_t width, uint32_t height, uint32_t mipCount);
int sceGxmTextureSetFormat(SceGxmTexture *texture, uint32_t format);
int sceGxmTextureSetWidth(SceGxmTexture *texture, uint32_t width);
int sceGxmTextureSetHeight(SceGxmTexture *texture, uint32_t height);
int sceGxmTextureSetData(SceGxmTexture *texture, const void *data);
int sceGxmTextureSetMipFilter(SceGxmTexture *texture, uint32_t filter);
int sceGxmTextureSetMinFilter(SceGxmTexture *texture, uint32_t filter);
int sceGxmTextureSetMagFilter(SceGxmTexture *texture, uint32_t filter);
int sceGxmTextureSetUAddrMode(SceGxmTexture *texture, uint32_t mode);
int sceGxmTextureSetVAddrMode(SceGxmTexture *texture, uint32_t mode);
int sceGxmTextureSetLodBias(SceGxmTexture *texture, uint32_t bias);

// Surface functions
int sceGxmColorSurfaceInit(SceGxmColorSurface *surface, uint32_t colorFormat,
                         uint32_t surfaceType, uint32_t colorSwizzle,
                         uint32_t outputRegisterSize, uint32_t width,
                         uint32_t height, uint32_t strideInPixels,
                         void *data);
int sceGxmDepthStencilSurfaceInit(SceGxmDepthStencilSurface *surface,
                                uint32_t depthStencilFormat,
                                uint32_t surfaceType,
                                uint32_t depthStencilSwizzle,
                                uint32_t width, uint32_t height,
                                uint32_t strideInSamples,
                                void *depthData, void *stencilData);

// Shader functions
const SceGxmProgram *sceGxmShaderPatcherGetProgramFromId(SceGxmShaderPatcherId programId);
uint32_t sceGxmProgramGetSize(const SceGxmProgram *program);
uint32_t sceGxmProgramGetType(const SceGxmProgram *program);
uint32_t sceGxmProgramGetParameterCount(const SceGxmProgram *program);
const SceGxmProgramParameter *sceGxmProgramGetParameter(const SceGxmProgram *program, uint32_t index);
const SceGxmProgramParameter *sceGxmProgramFindParameterByName(const SceGxmProgram *program, const char *name);
const SceGxmProgramParameter *sceGxmProgramFindParameterBySemantic(const SceGxmProgram *program, uint32_t semantic);
uint32_t sceGxmProgramParameterGetIndex(const SceGxmProgram *program, const SceGxmProgramParameter *parameter);
const char *sceGxmProgramParameterGetName(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetSemantic(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetSemanticIndex(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetCategory(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetComponentCount(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetArraySize(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetResourceIndex(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetContainerIndex(const SceGxmProgramParameter *parameter);
uint32_t sceGxmProgramParameterGetType(const SceGxmProgramParameter *parameter);

// Shader parameter structure
struct SceGxmProgramParameter {
    uint16_t category;
    uint16_t type;
    uint16_t componentCount;
    uint16_t arraySize;
    uint32_t resourceIndex;
    uint32_t containerIndex;
    char name[64];
};

// Global GXM manager instance
extern SceGxmManager g_gxm_manager;

} // namespace graphics
} // namespace modules
} // namespace firmware
