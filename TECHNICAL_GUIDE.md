# VitaNS Technical Guide

This document provides in-depth technical information about VitaNS's architecture, Mesa integration, and advanced development topics.

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Mesa Integration Deep Dive](#mesa-integration-deep-dive)
3. [Graphics Pipeline](#graphics-pipeline)
4. [Memory Management](#memory-management)
5. [CPU Emulation](#cpu-emulation)
6. [Build System Details](#build-system-details)
7. [Performance Considerations](#performance-considerations)
8. [Debugging and Profiling](#debugging-and-profiling)

## Architecture Overview

### System Design Philosophy

VitaNS employs a modular architecture that separates concerns between:
- **Hardware Emulation Layer**: Low-level system emulation
- **Graphics Abstraction Layer**: Mesa/OpenGL ES bridge
- **High-Level Emulation**: Game-specific optimizations
- **Platform Integration**: Nintendo Switch native features

### Core Components Interaction

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   PS Vita Game  │    │   VitaNS Core   │    │ Nintendo Switch │
│                 │    │                 │    │                 │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ GXM Calls   │◄┼────┼►│ GXM Emulator│ │    │ │ Mesa/EGL    │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ ARM Code    │◄┼────┼►│CPU Backend  │ │    │ │ libnx APIs  │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
│ ┌─────────────┐ │    │ ┌─────────────┐ │    │ ┌─────────────┐ │
│ │ System Calls│◄┼────┼►│Module System│ │    │ │ Switch HW   │ │
│ └─────────────┘ │    │ └─────────────┘ │    │ └─────────────┘ │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## Mesa Integration Deep Dive

### Custom Mesa Build Configuration

Our Mesa build is specifically optimized for Nintendo Switch:

```bash
# Key Mesa build options
meson setup build-switch \
    --cross-file=switch-cross.txt \
    -Dplatforms=switch \
    -Dgallium-drivers=softpipe \
    -Degl-native-platform=switch \
    -Dglx=disabled \
    -Dstrip=false \
    -Ddebug=false \
    -Doptimization=2
```

### Static Library Components

| Library | Purpose | Key Symbols |
|---------|---------|-------------|
| `libEGL.a` | EGL API implementation | `_eglDriver`, `eglInitialize` |
| `libGLESv2.a` | OpenGL ES 2.0 API | `glClear`, `glDrawArrays` |
| `libglapi_static.a` | GL dispatch table | `_glapi_get_proc_address` |
| `libmesa_util.a` | Utility functions | `util_call_once_data_slow` |
| `libsoftpipe.a` | Software rasterizer | Rendering pipeline |
| `libblake3.a` | Hash functions | Shader caching |
| `libmesa.a` | Core Mesa functionality | Driver infrastructure |

### Symbol Resolution Strategy

#### Critical Symbol Issues Resolved

1. **`_eglDriver` Undefined Reference**
   ```cmake
   # Solution: Force inclusion of all EGL objects
   -Wl,--whole-archive "${CUSTOM_MESA_LIB_DIR}/libEGL.a" -Wl,--no-whole-archive
   ```

2. **`_glapi_get_proc_address` Missing**
   ```cmake
   # Solution: Proper library ordering and grouping
   -Wl,--start-group
   libEGL.a libGLESv2.a libglapi_static.a ...
   -Wl,--end-group
   ```

3. **Multiple Definition Conflicts**
   ```cmake
   # Solution: Allow multiple definitions for GL functions
   -Wl,--allow-multiple-definition
   ```

### Library Linking Order

The order matters for static library resolution:

```cmake
# Optimized linking order
libEGL.a          # EGL entry points
libGLESv2.a       # OpenGL ES API
libmesa_util.a    # Utilities (called by above)
libsoftpipe.a     # Renderer backend
libblake3.a       # Hash utilities
libmesa.a         # Core infrastructure
libglapi_static.a # GL dispatch (resolves everything)
```

## Graphics Pipeline

### PS Vita GXM to OpenGL ES Translation

#### Command Buffer System

```cpp
// GXM command → OpenGL ES translation
class GxmCommandBuffer {
    struct UploadVertexBufferCommand {
        void* data;
        size_t size;
        uint32_t binding;
    };
    
    struct DrawCommand {
        GLenum primitive_type;
        uint32_t vertex_count;
        uint32_t index_count;
    };
    
    void add(std::unique_ptr<Command> cmd);
    void execute(RendererGLES2* renderer);
};
```

#### Shader Translation Pipeline

```
PS Vita Shader (GXP) → Shader Parser → GLSL ES → OpenGL ES Shader
```

Key challenges:
- **Register Allocation**: Vita's different register model
- **Precision Qualifiers**: GLSL ES requirements
- **Built-in Variables**: Mapping Vita semantics to GL

#### Texture and Framebuffer Management

```cpp
// Vita texture formats → OpenGL ES formats
struct TextureFormatMapping {
    SceGxmTextureFormat vita_format;
    GLenum gl_internal_format;
    GLenum gl_format;
    GLenum gl_type;
};

// Example mappings
{ SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE }
{ SCE_GXM_TEXTURE_FORMAT_F16F16F16F16_ABGR, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT }
```

### EGL Native Window Integration

#### Switch Native Window Structure

```cpp
typedef struct switch_native_window {
    void* framebuffer_ptr;     // Direct framebuffer access
    uint32_t stride_bytes;     // Scanline stride
    uint32_t width, height;    // Dimensions
    NWindow* nwin;             // libnx window handle
    Framebuffer fb;            // Framebuffer context
} switch_native_window;
```

#### EGL Surface Creation Flow

```cpp
// EGL surface creation for Switch
EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
eglInitialize(display, NULL, NULL);

EGLConfig config;
eglChooseConfig(display, config_attribs, &config, 1, &num_configs);

EGLSurface surface = eglCreateWindowSurface(display, config, 
    (EGLNativeWindowType)&switch_window, NULL);
    
EGLContext context = eglCreateContext(display, config, 
    EGL_NO_CONTEXT, context_attribs);
```

## Memory Management

### Address Space Layout

```
PS Vita Memory Layout          VitaNS Emulated Layout
┌─────────────────────┐       ┌─────────────────────┐
│ 0x80000000 - User  │  →    │ Switch User Memory  │
│ 0x90000000 - CDRAM │  →    │ Emulated CDRAM      │
│ 0xA0000000 - Kernel│  →    │ Emulated Kernel     │
└─────────────────────┘       └─────────────────────┘
```

### Memory Pool Management

```cpp
class MemoryManager {
    struct MemoryPool {
        void* base_address;
        size_t size;
        std::bitset<4096> allocation_bitmap;
    };
    
    MemoryPool user_pool;    // Main game memory
    MemoryPool cdram_pool;   // Graphics memory
    MemoryPool kernel_pool;  // System memory
};
```

## CPU Emulation

### ARM Cortex-A9 Emulation Strategy

VitaNS uses a hybrid approach:
- **High-Frequency Functions**: JIT compilation
- **System Calls**: High-level emulation
- **Critical Loops**: Optimized native code paths

#### JIT Compilation Pipeline

```cpp
class SwitchCpuBackend {
    // ARM → AArch64 translation
    void compile_block(uint32_t vita_pc, uint32_t* vita_code, size_t length);
    
    // Execution engine
    void execute_block(CompiledBlock* block);
    
    // Cache management
    std::unordered_map<uint32_t, CompiledBlock> block_cache;
};
```

## Build System Details

### CMake Configuration Highlights

#### Custom Mesa Integration
```cmake
# Mesa library discovery and linking
set(CUSTOM_MESA_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/lib")
set(CUSTOM_MESA_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/include")

# Priority include order
include_directories(BEFORE ${CUSTOM_MESA_INCLUDE_DIR})

# Linker groups for circular dependencies
target_link_libraries(VitaNS
    -Wl,--start-group
    ${MESA_LIBRARIES}
    -Wl,--end-group
)
```

#### Compiler Optimizations
```cmake
# C++20 features for modern code
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Switch-specific optimizations
target_compile_options(VitaNS PRIVATE
    -O2                    # Balanced optimization
    -ffast-math           # Math optimizations
    -march=armv8-a+crc    # Tegra X1 specific
)
```

### Dependency Management

#### Static Library Packaging
```bash
# Archive conversion for compatibility
for lib in *.a; do
    if ar t "$lib" | grep -q "^/"; then
        echo "Converting thin archive: $lib"
        ar -t "$lib" | xargs ar -rcs "${lib}.new"
        mv "${lib}.new" "$lib"
    fi
done
```

## Performance Considerations

### Graphics Performance

1. **Batching**: Minimize OpenGL ES state changes
2. **Caching**: Shader compilation and texture upload
3. **Memory**: Efficient buffer management
4. **Threading**: Separate render and emulation threads

### CPU Performance

1. **JIT Optimization**: Hot path identification
2. **System Call Caching**: Avoid repeated kernel emulation
3. **Memory Mapping**: Efficient address translation
4. **Instruction Prediction**: Branch optimization

### Memory Performance

1. **Pool Allocation**: Reduce malloc/free overhead
2. **Cache Alignment**: Optimize for Cortex-A57
3. **Prefetching**: Predictive memory access
4. **Compression**: Reduce memory footprint

## Debugging and Profiling

### Debug Build Configuration

```cmake
# Debug-specific settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(VitaNS PRIVATE
        VITANS_DEBUG=1
        MESA_DEBUG=1
        EGL_DEBUG=1
    )
    
    target_compile_options(VitaNS PRIVATE
        -g3 -O0 -fno-omit-frame-pointer
    )
endif()
```

### Logging System

```cpp
// Hierarchical logging
enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

#define LOG_GXM_DEBUG(fmt, ...) \
    if (log_level <= LogLevel::DEBUG) \
        printf("[GXM] " fmt "\n", ##__VA_ARGS__)
```

### Performance Profiling

```cpp
// Built-in profiler
class Profiler {
    struct Timer {
        std::chrono::high_resolution_clock::time_point start;
        std::string name;
    };
    
    static void begin_frame();
    static void end_frame();
    static void mark(const std::string& name);
};
```

### Memory Debugging

```cpp
// Memory leak detection
class MemoryTracker {
    std::unordered_map<void*, AllocationInfo> allocations;
    
public:
    void* tracked_malloc(size_t size, const char* file, int line);
    void tracked_free(void* ptr);
    void report_leaks();
};

#ifdef VITANS_DEBUG
#define MALLOC(size) MemoryTracker::tracked_malloc(size, __FILE__, __LINE__)
#define FREE(ptr) MemoryTracker::tracked_free(ptr)
#endif
```

## Advanced Topics

### Custom Mesa Driver Development

For developers wanting to extend Mesa support:

1. **Driver Interface**: Implement `struct pipe_screen`
2. **Context Management**: Handle OpenGL ES contexts
3. **Resource Management**: Textures, buffers, shaders
4. **Command Submission**: Efficient GPU communication

### Game-Specific Optimizations

```cpp
// Game detection and optimization
class GameDatabase {
    struct GameProfile {
        std::string title_id;
        std::vector<Optimization> optimizations;
        CompatibilityLevel compatibility;
    };
    
    GameProfile detect_game(const std::string& title_id);
    void apply_optimizations(const GameProfile& profile);
};
```

### Future Enhancements

1. **Vulkan Backend**: Next-generation graphics API
2. **Multi-threading**: Parallel execution improvements
3. **Dynamic Recompilation**: Better JIT performance
4. **Hardware Acceleration**: Tegra X1 specific optimizations

---

This technical guide provides the foundation for understanding and contributing to VitaNS development. For specific implementation details, refer to the source code and inline documentation.
