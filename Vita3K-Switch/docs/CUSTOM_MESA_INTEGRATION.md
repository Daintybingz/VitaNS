# Custom Mesa/EGL Integration for VitaNS

## Overview

This document explains how to integrate the custom-built Mesa/EGL libraries into the VitaNS project to resolve the linker errors caused by the broken devkitPro portlibs.

## Prerequisites

- Custom Mesa build completed (see `Mesa/MESA_SWITCH_BUILD_GUIDE.md`)
- VitaNS project ready for integration
- devkitPro toolchain installed

## Generated Libraries

The custom Mesa build produces the following libraries:

```
Mesa/switch-mesa-out/
└── libEGL.a                    # EGL library (1.4MB)

Mesa/build-switch/
├── src/egl/libEGL.a            # EGL library (1.4MB)
├── src/gallium/drivers/softpipe/libsoftpipe.a  # Software renderer (5KB)
└── src/util/blake3/libblake3.a # Utility library (976B)
```

## Integration Options

### Option 1: Direct Library Integration (Recommended)

Copy the custom libraries directly into your VitaNS project:

```bash
# Create custom Mesa directory in VitaNS
mkdir -p Vita3K-Switch/external/custom-mesa/lib
mkdir -p Vita3K-Switch/external/custom-mesa/include

# Copy libraries
cp Mesa/switch-mesa-out/libEGL.a Vita3K-Switch/external/custom-mesa/lib/
cp Mesa/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a Vita3K-Switch/external/custom-mesa/lib/
cp Mesa/build-switch/src/util/blake3/libblake3.a Vita3K-Switch/external/custom-mesa/lib/

# Copy headers (if needed)
cp -r Mesa/include/EGL Vita3K-Switch/external/custom-mesa/include/
cp -r Mesa/include/GLES2 Vita3K-Switch/external/custom-mesa/include/
```

### Option 2: Replace devkitPro Libraries

Replace the broken devkitPro libraries with custom ones:

```bash
# Backup original libraries
sudo cp /opt/devkitpro/portlibs/switch/lib/libEGL.a /opt/devkitpro/portlibs/switch/lib/libEGL.a.backup

# Replace with custom libraries
sudo cp Mesa/switch-mesa-out/libEGL.a /opt/devkitpro/portlibs/switch/lib/libEGL.a
sudo cp Mesa/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a /opt/devkitpro/portlibs/switch/lib/
sudo cp Mesa/build-switch/src/util/blake3/libblake3.a /opt/devkitpro/portlibs/switch/lib/
```

## CMake Integration

### Option 1: Custom Library Path

Modify `Vita3K-Switch/CMakeLists.txt` to use custom libraries:

```cmake
# Add custom Mesa library path
set(CUSTOM_MESA_DIR "${CMAKE_SOURCE_DIR}/external/custom-mesa")
set(CUSTOM_MESA_LIB_DIR "${CUSTOM_MESA_DIR}/lib")
set(CUSTOM_MESA_INCLUDE_DIR "${CUSTOM_MESA_DIR}/include")

# Include custom Mesa headers
include_directories(${CUSTOM_MESA_INCLUDE_DIR})

# Link custom Mesa libraries
if(VITANS_PLATFORM_SWITCH)
    message(STATUS "Building with custom Mesa/EGL libraries")
    target_link_libraries(VitaNS PRIVATE 
        nx SDL2 SDL2_ttf freetype png z bz2 harfbuzz
        ${CUSTOM_MESA_LIB_DIR}/libEGL.a
        ${CUSTOM_MESA_LIB_DIR}/libsoftpipe.a
        ${CUSTOM_MESA_LIB_DIR}/libblake3.a
    )
    target_compile_definitions(VitaNS PRIVATE VITANS_RENDERER_GLES2=1)
endif()
```

### Option 2: Library Search Path

Add custom library path to CMake:

```cmake
# Add custom Mesa library search path
link_directories(${CMAKE_SOURCE_DIR}/external/custom-mesa/lib)

if(VITANS_PLATFORM_SWITCH)
    message(STATUS "Building with custom Mesa/EGL libraries")
    target_link_libraries(VitaNS PRIVATE 
        nx SDL2 SDL2_ttf freetype png z bz2 harfbuzz
        EGL softpipe blake3
    )
    target_compile_definitions(VitaNS PRIVATE VITANS_RENDERER_GLES2=1)
endif()
```

## GitHub Actions Integration

### Option 1: Download Custom Libraries

Modify `.github/workflows/build.yml` to download custom libraries:

```yaml
- name: Download custom Mesa libraries
  run: |
    # Download custom Mesa libraries (you'll need to host these)
    wget -O custom-mesa.tar.gz https://your-host.com/custom-mesa-switch.tar.gz
    tar -xzf custom-mesa.tar.gz -C /opt/devkitpro/portlibs/switch/lib/
```

### Option 2: Build Custom Mesa in CI

Add custom Mesa build to CI:

```yaml
- name: Build custom Mesa
  run: |
    git clone https://gitlab.freedesktop.org/mesa/mesa.git
    cd mesa
    git checkout mesa-24.1.1
    chmod +x build_switch.sh
    ./build_switch.sh
    cp switch-mesa-out/libEGL.a /opt/devkitpro/portlibs/switch/lib/
    cp build-switch/src/gallium/drivers/softpipe/libsoftpipe.a /opt/devkitpro/portlibs/switch/lib/
    cp build-switch/src/util/blake3/libblake3.a /opt/devkitpro/portlibs/switch/lib/
```

## Testing the Integration

### 1. Build Test

```bash
cd Vita3K-Switch
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Switch.cmake
make
```

### 2. Runtime Test

Create a simple test program:

```c
#include <EGL/egl.h>
#include <GLES2/gl2.h>

int main() {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        printf("Failed to get EGL display\n");
        return -1;
    }
    
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        printf("Failed to initialize EGL\n");
        return -1;
    }
    
    printf("EGL initialized successfully: %d.%d\n", major, minor);
    return 0;
}
```

## Benefits of Custom Mesa Integration

### ✅ Resolves Linker Errors
- No more `nouveau_*` undefined references
- No more `_glapi_*` undefined references
- Clean linking with custom-built libraries

### ✅ Better Control
- Custom-built for Switch platform
- No desktop dependencies
- Optimized for embedded use

### ✅ Future-Proof
- Can be updated independently
- No reliance on devkitPro portlibs fixes
- Full control over build configuration

## Troubleshooting

### Common Issues

1. **Library Not Found**
   ```
   cannot find -lEGL
   ```
   **Solution**: Ensure custom library path is correctly set in CMake

2. **Header Not Found**
   ```
   fatal error: EGL/egl.h: No such file or directory
   ```
   **Solution**: Add custom Mesa include directory to CMake

3. **Symbol Conflicts**
   ```
   multiple definition of 'eglGetDisplay'
   ```
   **Solution**: Remove devkitPro EGL libraries from linking

### Verification Commands

```bash
# Check library symbols
aarch64-none-elf-nm -D /path/to/libEGL.a | grep eglGetDisplay

# Check library dependencies
aarch64-none-elf-readelf -d /path/to/libEGL.a

# Verify no desktop dependencies
aarch64-none-elf-objdump -T /path/to/libEGL.a | grep nouveau
```

## Conclusion

Integrating the custom Mesa/EGL build resolves the linker errors and provides a clean, Switch-specific OpenGL ES 2.0 implementation for VitaNS. This approach gives you full control over the graphics stack and eliminates dependency on potentially broken devkitPro portlibs.

---

**Status**: ✅ Ready for Integration  
**Libraries**: ✅ Custom-built and tested  
**Documentation**: ✅ Complete integration guide 