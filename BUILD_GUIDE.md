# VitaNS Build Guide

This comprehensive guide will walk you through building VitaNS from source, including the custom Mesa integration and all dependencies.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Environment Setup](#environment-setup)
3. [Mesa Compilation](#mesa-compilation)
4. [VitaNS Compilation](#vitans-compilation)
5. [CI/CD Pipeline](#cicd-pipeline)
6. [Development Workflow](#development-workflow)
7. [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware Requirements
- **Windows 10/11** (primary development) or **Linux Ubuntu 20.04+**
- **16GB RAM minimum** (32GB recommended for Mesa compilation)
- **50GB free disk space** (for toolchains, Mesa build artifacts, and VitaNS)
- **Fast internet connection** (for downloading large toolchain packages)

### Software Requirements
- **Git** with LFS support
- **WSL2** (if developing on Windows)
- **Visual Studio Code** or equivalent IDE
- **Python 3.8+** and **pip**

## Environment Setup

### Option 1: Windows with WSL2 (Recommended)

#### 1. Install WSL2 with Ubuntu
```powershell
# Run as Administrator
wsl --install -d Ubuntu-20.04
wsl --set-default-version 2
```

#### 2. Configure WSL2 Ubuntu Environment
```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install build essentials
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    python3-meson \
    python3-pip \
    pkg-config \
    git \
    git-lfs \
    curl \
    wget \
    unzip

# Install Python dependencies
pip3 install mako jinja2
```

#### 3. Install DevkitPro Toolchain
```bash
# Add DevkitPro repository
wget https://apt.devkitpro.org/install-devkitpro-pacman
chmod +x ./install-devkitpro-pacman
sudo ./install-devkitpro-pacman

# Install Switch development tools
sudo dkp-pacman -S switch-dev

# Set environment variables
echo 'export DEVKITPRO=/opt/devkitpro' >> ~/.bashrc
echo 'export DEVKITARM=/opt/devkitpro/devkitARM' >> ~/.bashrc
echo 'export DEVKITA64=/opt/devkitpro/devkitA64' >> ~/.bashrc
echo 'export PATH=$PATH:$DEVKITPRO/tools/bin' >> ~/.bashrc
source ~/.bashrc
```

### Option 2: Pure Linux Environment

```bash
# Ubuntu/Debian setup
sudo apt update && sudo apt upgrade -y

# Install dependencies
sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    meson \
    python3-mako \
    python3-jinja2 \
    pkg-config \
    git \
    git-lfs \
    curl \
    wget \
    unzip \
    bison \
    flex \
    gettext

# Install DevkitPro (same as WSL2 steps above)
```

### Option 3: GitHub Codespaces (Cloud Development)

```yaml
# .devcontainer/devcontainer.json
{
    "name": "VitaNS Development",
    "image": "ubuntu:20.04",
    "features": {
        "ghcr.io/devcontainers/features/git:1": {},
        "ghcr.io/devcontainers/features/python:1": {"version": "3.8"}
    },
    "postCreateCommand": ".devcontainer/setup.sh"
}
```

## Mesa Compilation

### 1. Clone Mesa Repository

```bash
# Navigate to your project directory
cd ~/VitaNS  # or wherever you keep your projects

# Clone Mesa with submodules
git clone --depth=1 --branch=main https://gitlab.freedesktop.org/mesa/mesa.git
cd mesa
git submodule update --init --depth=1
```

### 2. Create Switch Cross-Compilation File

```bash
# Create Mesa/cross-files/switch.txt
mkdir -p cross-files
cat > cross-files/switch.txt << 'EOF'
[binaries]
c = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc'
cpp = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++'
ar = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar'
strip = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-strip'
pkgconfig = 'pkg-config'

[host_machine]
system = 'switch'
cpu_family = 'aarch64'
cpu = 'cortex-a57'
endian = 'little'

[properties]
c_args = ['-D__SWITCH__', '-I/opt/devkitpro/libnx/include', '-I/opt/devkitpro/portlibs/switch/include']
cpp_args = ['-D__SWITCH__', '-I/opt/devkitpro/libnx/include', '-I/opt/devkitpro/portlibs/switch/include']
c_link_args = ['-L/opt/devkitpro/libnx/lib', '-L/opt/devkitpro/portlibs/switch/lib']
cpp_link_args = ['-L/opt/devkitpro/libnx/lib', '-L/opt/devkitpro/portlibs/switch/lib']
EOF
```

### 3. Configure Mesa Build

```bash
# Create build directory
mkdir -p build-switch
cd build-switch

# Configure with Meson
meson setup . .. \
    --cross-file=../cross-files/switch.txt \
    --buildtype=release \
    -Dstrip=false \
    -Ddebug=false \
    -Doptimization=2 \
    -Dplatforms=switch \
    -Dgallium-drivers=softpipe \
    -Dvulkan-drivers= \
    -Degl-native-platform=switch \
    -Dglx=disabled \
    -Dgbm=disabled \
    -Dllvm=disabled \
    -Dshared-llvm=disabled \
    -Dvalgrind=disabled \
    -Dlibunwind=disabled \
    -Dlmsensors=disabled \
    -Dbuild-tests=false \
    -Dinstall-intel-gpu-tests=false \
    -Dselinux=false \
    -Dosmesa=false \
    -Dgallium-nine=false \
    -Dgallium-xa=false \
    -Dgallium-vdpau=disabled \
    -Dgallium-va=disabled \
    -Dgallium-omx=disabled \
    -Dgles1=disabled \
    -Dgles2=enabled \
    -Dopengl=disabled \
    -Dshared-glapi=disabled \
    -Dzstd=disabled
```

### 4. Compile Mesa

```bash
# Build Mesa (this will take 30-60 minutes)
ninja -j$(nproc)

# Verify build success
ls src/egl/*.a src/mapi/es2api/*.a src/util/*.a src/gallium/drivers/softpipe/*.a
```

### 5. Copy Mesa Libraries to VitaNS

```bash
# Navigate to VitaNS project root
cd ~/VitaNS  # Adjust path as needed

# Create directory structure
mkdir -p Vita3K-Switch/external/custom-mesa/{lib,include}

# Copy static libraries
cp mesa/build-switch/src/egl/libEGL.a Vita3K-Switch/external/custom-mesa/lib/
cp mesa/build-switch/src/mapi/es2api/libGLESv2.a Vita3K-Switch/external/custom-mesa/lib/
cp mesa/build-switch/src/mapi/shared-glapi/libglapi_static.a Vita3K-Switch/external/custom-mesa/lib/
cp mesa/build-switch/src/util/libmesa_util.a Vita3K-Switch/external/custom-mesa/lib/
cp mesa/build-switch/src/gallium/drivers/softpipe/libsoftpipe.a Vita3K-Switch/external/custom-mesa/lib/
cp mesa/build-switch/subprojects/blake3-1.3.1/libblake3.a Vita3K-Switch/external/custom-mesa/lib/
cp mesa/build-switch/src/mesa/libmesa.a Vita3K-Switch/external/custom-mesa/lib/

# Copy headers
cp -r mesa/include/* Vita3K-Switch/external/custom-mesa/include/
cp -r mesa/build-switch/src/util/format/u_format_pack.h Vita3K-Switch/external/custom-mesa/include/
cp -r mesa/build-switch/src/compiler/nir/nir_opcodes.h Vita3K-Switch/external/custom-mesa/include/

# Copy platform-specific headers
mkdir -p Vita3K-Switch/external/custom-mesa/include/platform
cp mesa/src/egl/drivers/dri2/platform_switch.h Vita3K-Switch/external/custom-mesa/include/platform/
```

### 6. Create Switch-Specific Headers

```bash
# Create stub dlfcn.h for Switch
cat > Vita3K-Switch/external/custom-mesa/include/dlfcn.h << 'EOF'
#ifndef DLFCN_H_SWITCH_STUB
#define DLFCN_H_SWITCH_STUB

// Nintendo Switch does not support dynamic linking
// This is a stub implementation for compilation compatibility

#define RTLD_LAZY    0x00001
#define RTLD_NOW     0x00002
#define RTLD_GLOBAL  0x00100
#define RTLD_LOCAL   0x00000

static inline void* dlopen(const char* filename, int flag) {
    (void)filename; (void)flag;
    return nullptr;
}

static inline void* dlsym(void* handle, const char* symbol) {
    (void)handle; (void)symbol;
    return nullptr;
}

static inline int dlclose(void* handle) {
    (void)handle;
    return 0;
}

static inline char* dlerror(void) {
    return (char*)"Dynamic linking not supported on Switch";
}

#endif // DLFCN_H_SWITCH_STUB
EOF

# Create switch_native_window.h
cat > Vita3K-Switch/external/custom-mesa/include/switch_native_window.h << 'EOF'
#ifndef SWITCH_NATIVE_WINDOW_H
#define SWITCH_NATIVE_WINDOW_H

#include <switch.h>

typedef struct switch_native_window {
    void* framebuffer_ptr;
    uint32_t stride_bytes;
    uint32_t width;
    uint32_t height;
    NWindow* nwin;
    Framebuffer fb;
} switch_native_window;

#endif // SWITCH_NATIVE_WINDOW_H
EOF
```

## VitaNS Compilation

### 1. Clone VitaNS Repository

```bash
# Clone VitaNS (adjust URL to your fork/repository)
git clone https://github.com/your-username/VitaNS.git
cd VitaNS

# Initialize Git LFS (for large binary files)
git lfs install
git lfs pull
```

### 2. Verify Mesa Integration

```bash
# Check that Mesa libraries exist
ls -la Vita3K-Switch/external/custom-mesa/lib/
# Should show 7 .a files: libEGL.a, libGLESv2.a, libglapi_static.a, 
# libmesa_util.a, libsoftpipe.a, libblake3.a, libmesa.a

# Check headers
ls -la Vita3K-Switch/external/custom-mesa/include/
# Should show EGL/, GLES2/, GL/, etc.
```

### 3. Configure Build Environment

```bash
# Ensure DevkitPro environment is set
source ~/.bashrc

# Verify toolchain
which aarch64-none-elf-gcc
/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc

# Check libnx
ls $DEVKITPRO/libnx/
```

### 4. Build VitaNS

```bash
# Navigate to build directory
cd Vita3K-Switch

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20

# Build (this will take 10-15 minutes)
make -j$(nproc)

# Verify successful build
ls -la VitaNS.nro
# Should show ~56MB file
```

### 5. Test Build Locally (Optional)

```bash
# Basic file validation
file VitaNS.nro
# Should show: Nintendo Switch homebrew executable

# Size check
du -h VitaNS.nro
# Should show ~56M

# Symbol verification (ensure no undefined references)
readelf -s VitaNS.nro | grep UND | wc -l
# Should show 0 or very few undefined symbols
```

## CI/CD Pipeline

### GitHub Actions Workflow

Our automated build process in `.github/workflows/build.yml`:

#### Key Features
- **Automatic Triggering**: Builds on every push to master
- **Environment Setup**: DevkitPro installation and configuration  
- **Mesa Verification**: Checks for all required libraries and symbols
- **Build Validation**: Ensures VitaNS.nro compiles successfully
- **Artifact Upload**: Saves build outputs for download

#### Workflow Steps
1. **Environment Setup**
   ```yaml
   - name: Install DevkitPro
     run: |
       wget https://apt.devkitpro.org/install-devkitpro-pacman
       sudo ./install-devkitpro-pacman
       sudo dkp-pacman -S switch-dev
   ```

2. **Mesa Verification**
   ```yaml
   - name: Verify custom Mesa libraries exist
     run: |
       for lib in libEGL.a libGLESv2.a libglapi_static.a libmesa_util.a libsoftpipe.a libblake3.a libmesa.a; do
         if [ ! -f "Vita3K-Switch/external/custom-mesa/lib/$lib" ]; then
           echo "Missing required Mesa library: $lib"
           exit 1
         fi
       done
   ```

3. **Symbol Verification**
   ```yaml
   - name: Verify Mesa library symbols
     run: |
       # Check for critical symbols
       nm Vita3K-Switch/external/custom-mesa/lib/libglapi_static.a | grep -q "_glapi_get_proc_address"
       nm Vita3K-Switch/external/custom-mesa/lib/libEGL.a | grep -q "_eglDriver"
   ```

4. **Build Process**
   ```yaml
   - name: Build VitaNS
     run: |
       cd Vita3K-Switch
       mkdir build && cd build
       cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
       make -j$(nproc)
   ```

### Local CI Testing

```bash
# Simulate CI environment locally
docker run -it --rm -v $(pwd):/workspace ubuntu:20.04

# Inside container, run CI steps
apt update && apt upgrade -y
# ... follow CI workflow steps
```

## Development Workflow

### 1. Setting Up Development Environment

```bash
# VS Code with useful extensions
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension ms-vscode-remote.remote-wsl

# Configure VS Code settings
cat > .vscode/settings.json << 'EOF'
{
    "C_Cpp.default.compilerPath": "/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc",
    "C_Cpp.default.includePath": [
        "/opt/devkitpro/libnx/include",
        "/opt/devkitpro/portlibs/switch/include",
        "${workspaceFolder}/Vita3K-Switch/external/custom-mesa/include"
    ],
    "C_Cpp.default.cppStandard": "c++20",
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake"
    ]
}
EOF
```

### 2. Iterative Development

```bash
# Quick build script for development
cat > quick_build.sh << 'EOF'
#!/bin/bash
cd Vita3K-Switch/build
make -j$(nproc) && echo "✅ Build successful!" || echo "❌ Build failed!"
EOF
chmod +x quick_build.sh

# Quick test script
cat > quick_test.sh << 'EOF'
#!/bin/bash
./quick_build.sh
if [ $? -eq 0 ]; then
    ls -la Vita3K-Switch/build/VitaNS.nro
    du -h Vita3K-Switch/build/VitaNS.nro
fi
EOF
chmod +x quick_test.sh
```

### 3. Debugging Build Issues

```bash
# Verbose build for debugging
cd Vita3K-Switch/build
make VERBOSE=1

# Check linker command
make VERBOSE=1 2>&1 | grep -A 5 -B 5 "aarch64-none-elf-g++"

# Symbol debugging
nm --debug Vita3K-Switch/external/custom-mesa/lib/*.a | grep "undefined"
objdump -t VitaNS.nro | grep "NEEDED"
```

## Advanced Build Topics

### 1. Custom Mesa Modifications

If you need to modify Mesa source:

```bash
# Make changes to Mesa source
cd mesa
# ... edit files ...

# Rebuild specific components
cd build-switch
ninja src/egl/libEGL.a
ninja src/mapi/es2api/libGLESv2.a

# Copy updated libraries
cp src/egl/libEGL.a ../Vita3K-Switch/external/custom-mesa/lib/
cp src/mapi/es2api/libGLESv2.a ../Vita3K-Switch/external/custom-mesa/lib/
```

### 2. Optimization Builds

```bash
# Debug build for development
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake

# Release with debug info
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake

# Maximum optimization
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -flto" -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
```

### 3. Profiling Builds

```bash
# Build with profiling support
cmake .. \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_CXX_FLAGS="-pg -fno-omit-frame-pointer" \
    -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
```

## Build Scripts and Automation

### 1. Complete Build Script

```bash
cat > build_all.sh << 'EOF'
#!/bin/bash
set -e

echo "🚀 Starting VitaNS complete build process..."

# Check prerequisites
command -v git >/dev/null 2>&1 || { echo "❌ Git is required"; exit 1; }
command -v cmake >/dev/null 2>&1 || { echo "❌ CMake is required"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "❌ Make is required"; exit 1; }

# Check DevkitPro
if [ ! -d "/opt/devkitpro" ]; then
    echo "❌ DevkitPro not found. Please install it first."
    exit 1
fi

# Source environment
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export DEVKITA64=/opt/devkitpro/devkitA64
export PATH=$PATH:$DEVKITPRO/tools/bin

echo "✅ Environment configured"

# Verify Mesa libraries
MESA_LIB_DIR="Vita3K-Switch/external/custom-mesa/lib"
REQUIRED_LIBS=(libEGL.a libGLESv2.a libglapi_static.a libmesa_util.a libsoftpipe.a libblake3.a libmesa.a)

for lib in "${REQUIRED_LIBS[@]}"; do
    if [ ! -f "$MESA_LIB_DIR/$lib" ]; then
        echo "❌ Missing Mesa library: $lib"
        echo "Please run Mesa compilation first"
        exit 1
    fi
done

echo "✅ Mesa libraries verified"

# Build VitaNS
cd Vita3K-Switch
mkdir -p build
cd build

echo "🔨 Configuring build..."
cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake -DCMAKE_BUILD_TYPE=Release

echo "🔨 Building VitaNS..."
make -j$(nproc)

if [ -f "VitaNS.nro" ]; then
    echo "✅ Build successful!"
    echo "📊 VitaNS.nro size: $(du -h VitaNS.nro | cut -f1)"
    echo "📁 Output: $(pwd)/VitaNS.nro"
else
    echo "❌ Build failed!"
    exit 1
fi
EOF

chmod +x build_all.sh
```

### 2. Mesa Build Script

```bash
cat > Mesa/rebuild_and_copy.sh << 'EOF'
#!/bin/bash
set -e

echo "🚀 Rebuilding Mesa for Nintendo Switch..."

# Check prerequisites
command -v meson >/dev/null 2>&1 || { echo "❌ Meson is required"; exit 1; }
command -v ninja >/dev/null 2>&1 || { echo "❌ Ninja is required"; exit 1; }

# Check DevkitPro
if [ ! -d "/opt/devkitpro" ]; then
    echo "❌ DevkitPro not found"
    exit 1
fi

# Clean previous build
rm -rf build-complete

# Configure Mesa
echo "⚙️  Configuring Mesa..."
meson setup build-complete \
    --cross-file=cross-files/switch.txt \
    --buildtype=release \
    -Dstrip=false \
    -Ddebug=false \
    -Doptimization=2 \
    -Dplatforms=switch \
    -Dgallium-drivers=softpipe \
    -Degl-native-platform=switch \
    -Dglx=disabled \
    -Dgbm=disabled \
    -Dllvm=disabled \
    -Dshared-glapi=disabled \
    -Dgles2=enabled \
    -Dopengl=disabled

# Build Mesa
echo "🔨 Building Mesa..."
cd build-complete
ninja -j$(nproc)

echo "✅ Mesa build complete!"

# Copy libraries
echo "📁 Copying libraries to VitaNS..."
cd ..
mkdir -p ../Vita3K-Switch/external/custom-mesa/lib

cp build-complete/src/egl/libEGL.a ../Vita3K-Switch/external/custom-mesa/lib/
cp build-complete/src/mapi/es2api/libGLESv2.a ../Vita3K-Switch/external/custom-mesa/lib/
cp build-complete/src/mapi/shared-glapi/libglapi_static.a ../Vita3K-Switch/external/custom-mesa/lib/
cp build-complete/src/util/libmesa_util.a ../Vita3K-Switch/external/custom-mesa/lib/
cp build-complete/src/gallium/drivers/softpipe/libsoftpipe.a ../Vita3K-Switch/external/custom-mesa/lib/
cp build-complete/subprojects/blake3-*/libblake3.a ../Vita3K-Switch/external/custom-mesa/lib/
cp build-complete/src/mesa/libmesa.a ../Vita3K-Switch/external/custom-mesa/lib/

echo "✅ Libraries copied successfully!"

# Verify symbols
echo "🔍 Verifying critical symbols..."
nm ../Vita3K-Switch/external/custom-mesa/lib/libglapi_static.a | grep -q "_glapi_get_proc_address" && echo "✅ _glapi_get_proc_address found"
nm ../Vita3K-Switch/external/custom-mesa/lib/libEGL.a | grep -q "_eglDriver" && echo "✅ _eglDriver found"

echo "🎉 Mesa rebuild and copy completed successfully!"
EOF

chmod +x Mesa/rebuild_and_copy.sh
```

---

This build guide provides everything needed to compile VitaNS from source. For troubleshooting specific issues, see [TROUBLESHOOTING.md](TROUBLESHOOTING.md).
