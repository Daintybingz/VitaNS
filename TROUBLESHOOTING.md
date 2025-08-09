# VitaNS Troubleshooting Guide

This guide helps you resolve common issues encountered when building, developing, or running VitaNS.

## Table of Contents

1. [Build Issues](#build-issues)
2. [Mesa Compilation Problems](#mesa-compilation-problems)
3. [Linking Errors](#linking-errors)
4. [Runtime Issues](#runtime-issues)
5. [Development Environment](#development-environment)
6. [CI/CD Pipeline Issues](#cicd-pipeline-issues)
7. [Performance Issues](#performance-issues)
8. [Platform-Specific Issues](#platform-specific-issues)

## Build Issues

### ❌ CMake Configuration Fails

#### Problem: `CMAKE_TOOLCHAIN_FILE not found`
```
CMake Error: Could not find CMAKE_TOOLCHAIN_FILE
```

**Solution:**
```bash
# Verify DevkitPro installation
ls /opt/devkitpro/cmake/Switch.cmake

# If missing, reinstall DevkitPro
sudo dkp-pacman -S switch-dev

# Use absolute path
cmake .. -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake
```

#### Problem: `C++ compiler not working`
```
CMake Error: The C++ compiler is not able to compile a simple test program
```

**Solution:**
```bash
# Check compiler exists
which aarch64-none-elf-g++

# Verify environment variables
echo $DEVKITPRO
echo $DEVKITA64

# Reinstall toolchain if needed
sudo dkp-pacman -S devkitA64 libnx switch-tools
```

### ❌ C++20 Standard Issues

#### Problem: `lambda-expression in unevaluated context only available with '-std=c++20'`

**Solution:**
```cmake
# In CMakeLists.txt
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

#### Problem: `concepts` or other C++20 features not available

**Solution:**
```bash
# Check compiler version
aarch64-none-elf-g++ --version
# Should be GCC 11+ for full C++20 support

# Update DevkitPro if needed
sudo dkp-pacman -Syu
```

### ❌ Include Path Issues

#### Problem: `fatal error: 'EGL/egl.h' file not found`

**Solution:**
```bash
# Verify Mesa headers exist
ls Vita3K-Switch/external/custom-mesa/include/EGL/

# Check CMakeLists.txt include order
include_directories(BEFORE ${CUSTOM_MESA_INCLUDE_DIR})

# Verify path is correct
echo ${CUSTOM_MESA_INCLUDE_DIR}
```

#### Problem: `switch.h` not found

**Solution:**
```bash
# Check libnx installation
ls /opt/devkitpro/libnx/include/switch.h

# Verify environment
source ~/.bashrc
echo $DEVKITPRO

# Reinstall if needed
sudo dkp-pacman -S libnx
```

## Mesa Compilation Problems

### ❌ Meson Configuration Fails

#### Problem: `Cross-compilation file not found`

**Solution:**
```bash
# Create cross-compilation file
mkdir -p cross-files
cat > cross-files/switch.txt << 'EOF'
[binaries]
c = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc'
cpp = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-g++'
ar = '/opt/devkitpro/devkitA64/bin/aarch64-none-elf-ar'
# ... rest of file
EOF
```

#### Problem: `Python dependencies missing`

**Solution:**
```bash
# Install required Python packages
pip3 install mako jinja2 pyyaml

# For Ubuntu/Debian
sudo apt install python3-mako python3-jinja2
```

### ❌ Mesa Build Errors

#### Problem: `ninja: build stopped: subcommand failed`

**Solution:**
```bash
# Build with verbose output
ninja -v

# Build single-threaded to see errors clearly
ninja -j1

# Clean and rebuild
rm -rf build-switch
meson setup build-switch --cross-file=cross-files/switch.txt
```

#### Problem: `undefined reference to '__errno_location'`

**Solution:**
```bash
# Add to cross-file c_args
c_args = ['-D__SWITCH__', '-D_GNU_SOURCE', '-I/opt/devkitpro/libnx/include']
```

### ❌ Missing Mesa Libraries

#### Problem: Mesa libraries not generated

**Solution:**
```bash
# Check build output
ls build-switch/src/egl/
ls build-switch/src/mapi/es2api/

# Verify configuration
meson configure build-switch | grep -E "(egl|gles)"

# Ensure correct drivers enabled
-Dgallium-drivers=softpipe
-Dgles2=enabled
```

## Linking Errors

### ❌ Undefined Reference Errors

#### Problem: `undefined reference to '_glapi_get_proc_address'`

**Solution:**
```cmake
# Ensure proper library linking order
-Wl,--start-group
libEGL.a libGLESv2.a libglapi_static.a libmesa_util.a 
libsoftpipe.a libblake3.a libmesa.a
-Wl,--end-group
```

#### Problem: `undefined reference to '_eglDriver'`

**Solution:**
```cmake
# Force inclusion of all EGL objects
-Wl,--whole-archive "${CUSTOM_MESA_LIB_DIR}/libEGL.a" -Wl,--no-whole-archive
```

#### Problem: `multiple definition of 'glGenProgramPipelines'`

**Solution:**
```cmake
# Allow multiple definitions
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-multiple-definition")
```

### ❌ Library Not Found Errors

#### Problem: `No rule to make target 'libEGL.a'`

**Solution:**
```bash
# Verify library exists
ls Vita3K-Switch/external/custom-mesa/lib/libEGL.a

# Check CMake variable
echo ${CUSTOM_MESA_LIB_DIR}

# Verify path in CMakeLists.txt
set(CUSTOM_MESA_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/custom-mesa/lib")
```

## Runtime Issues

### ❌ EGL Initialization Fails

#### Problem: `eglInitialize() returns EGL_FALSE`

**Solution:**
```cpp
// Check EGL error
EGLint error = eglGetError();
printf("EGL Error: 0x%x\n", error);

// Verify display
EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
if (display == EGL_NO_DISPLAY) {
    printf("Failed to get EGL display\n");
}

// Check driver loading
printf("EGL Vendor: %s\n", eglQueryString(display, EGL_VENDOR));
```

### ❌ Black Screen Issues

#### Problem: VitaNS displays black screen

**Solution:**
```cpp
// Verify framebuffer
if (!ctx->framebuffer_ptr) {
    printf("Framebuffer not initialized\n");
}

// Check OpenGL state
GLenum gl_error = glGetError();
if (gl_error != GL_NO_ERROR) {
    printf("OpenGL Error: 0x%x\n", gl_error);
}

// Verify clear color
glClearColor(0.0f, 0.0f, 0.5f, 1.0f);  // Should show blue
glClear(GL_COLOR_BUFFER_BIT);
```

### ❌ Input Not Working

#### Problem: Buttons don't respond

**Solution:**
```cpp
// Check pad initialization
PadState pad;
padConfigureInput(1, HidNpadStyleSet_NpadStandard);
padInitializeDefault(&pad);

// Verify input reading
padUpdate(&pad);
u64 kDown = padGetButtonsDown(&pad);
printf("Buttons: 0x%llx\n", kDown);
```

## Development Environment

### ❌ WSL2 Issues

#### Problem: `WSL2 running slowly`

**Solution:**
```bash
# Check WSL2 version
wsl --version

# Optimize WSL2 settings
echo '[wsl2]
memory=16GB
processors=8' > ~/.wslconfig

# Restart WSL2
wsl --shutdown
```

#### Problem: `File permissions in WSL2`

**Solution:**
```bash
# Fix permissions
sudo chown -R $(whoami):$(whoami) /path/to/project

# Add to .bashrc for automount options
echo 'umask 022' >> ~/.bashrc
```

### ❌ VS Code Integration

#### Problem: `IntelliSense not working`

**Solution:**
```json
// .vscode/c_cpp_properties.json
{
    "configurations": [
        {
            "name": "Switch",
            "includePath": [
                "/opt/devkitpro/libnx/include",
                "${workspaceFolder}/Vita3K-Switch/external/custom-mesa/include"
            ],
            "compilerPath": "/opt/devkitpro/devkitA64/bin/aarch64-none-elf-gcc",
            "cStandard": "c17",
            "cppStandard": "c++20"
        }
    ]
}
```

## CI/CD Pipeline Issues

### ❌ GitHub Actions Failures

#### Problem: `DevkitPro installation fails`

**Solution:**
```yaml
# Use exact package manager commands
- name: Install DevkitPro
  run: |
    wget -O install-devkitpro-pacman https://apt.devkitpro.org/install-devkitpro-pacman
    chmod +x install-devkitpro-pacman
    sudo ./install-devkitpro-pacman
    sudo dkp-pacman -Sy
    sudo dkp-pacman -S switch-dev
```

#### Problem: `Build times out`

**Solution:**
```yaml
# Optimize build settings
- name: Build VitaNS
  timeout-minutes: 60  # Increase timeout
  run: |
    cd Vita3K-Switch/build
    make -j2  # Reduce parallelism for stability
```

#### Problem: `Artifact upload fails`

**Solution:**
```yaml
# Check file exists before upload
- name: Upload VitaNS.nro
  if: success()
  run: |
    if [ -f "Vita3K-Switch/build/VitaNS.nro" ]; then
      # Upload logic
    else
      echo "VitaNS.nro not found!"
      exit 1
    fi
```

### ❌ Shell Script Issues

#### Problem: `Syntax error: "(" unexpected`

**Solution:**
```bash
# Use POSIX-compatible syntax instead of bash arrays
# BAD:
LIBS=("libEGL.a" "libGLESv2.a")
for lib in "${LIBS[@]}"; do

# GOOD:
for lib in libEGL.a libGLESv2.a libglapi_static.a; do
    echo "Checking $lib"
done
```

## Performance Issues

### ❌ Slow Compilation

#### Problem: `Mesa takes hours to compile`

**Solution:**
```bash
# Use ccache
sudo apt install ccache
export CC="ccache gcc"
export CXX="ccache g++"

# Reduce optimization for faster builds
-Doptimization=1  # Instead of 2

# Use ninja instead of make
ninja -j$(nproc)
```

#### Problem: `VitaNS compilation is slow`

**Solution:**
```bash
# Use parallel compilation
make -j$(nproc)

# For development, use debug builds
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Incremental builds
make VitaNS  # Only build main target
```

### ❌ Runtime Performance

#### Problem: `Low FPS on Switch`

**Solution:**
```cpp
// Enable optimizations
#define VITANS_RENDERER_SOFTWARE  // Use software renderer
#define IMGUI_DISABLE_DEMO_WINDOWS  // Reduce overhead

// Optimize render loop
glDisable(GL_DEPTH_TEST);  // If not needed
glDisable(GL_BLEND);      // If not needed
```

## Platform-Specific Issues

### ❌ Windows Development

#### Problem: `Long path names in Windows`

**Solution:**
```powershell
# Enable long paths in Windows
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force

# Or use shortened paths
subst V: C:\Users\Username\VeryLongPathToProject
cd V:\
```

#### Problem: `Line ending issues`

**Solution:**
```bash
# Configure Git for consistent line endings
git config --global core.autocrlf input
git config --global core.eol lf

# Fix existing files
git rm --cached -r .
git reset --hard
```

### ❌ Linux-Specific Issues

#### Problem: `Permission denied on DevkitPro`

**Solution:**
```bash
# Add user to devkitpro group
sudo usermod -a -G devkitpro $USER

# Fix permissions
sudo chown -R root:devkitpro /opt/devkitpro
sudo chmod -R g+rw /opt/devkitpro

# Logout and login again
```

## Quick Diagnostic Commands

### Environment Check
```bash
# Check all requirements
echo "=== Environment Check ==="
echo "DevkitPro: $DEVKITPRO"
echo "libnx: $(ls $DEVKITPRO/libnx 2>/dev/null && echo "OK" || echo "MISSING")"
echo "Compiler: $(which aarch64-none-elf-gcc 2>/dev/null || echo "MISSING")"
echo "CMake: $(cmake --version | head -1)"
echo "Git: $(git --version)"
```

### Mesa Libraries Check
```bash
# Verify all Mesa libraries
echo "=== Mesa Libraries Check ==="
MESA_DIR="Vita3K-Switch/external/custom-mesa/lib"
for lib in libEGL.a libGLESv2.a libglapi_static.a libmesa_util.a libsoftpipe.a libblake3.a libmesa.a; do
    if [ -f "$MESA_DIR/$lib" ]; then
        echo "✅ $lib ($(du -h "$MESA_DIR/$lib" | cut -f1))"
    else
        echo "❌ $lib MISSING"
    fi
done
```

### Symbol Verification
```bash
# Check critical symbols
echo "=== Symbol Verification ==="
nm Vita3K-Switch/external/custom-mesa/lib/libglapi_static.a 2>/dev/null | grep -q "_glapi_get_proc_address" && echo "✅ _glapi_get_proc_address" || echo "❌ _glapi_get_proc_address MISSING"
nm Vita3K-Switch/external/custom-mesa/lib/libEGL.a 2>/dev/null | grep -q "_eglDriver" && echo "✅ _eglDriver" || echo "❌ _eglDriver MISSING"
```

### Build System Check
```bash
# Verify build environment
echo "=== Build System Check ==="
cd Vita3K-Switch/build 2>/dev/null || { echo "❌ Build directory missing"; exit 1; }
[ -f CMakeCache.txt ] && echo "✅ CMake configured" || echo "❌ CMake not configured"
[ -f Makefile ] && echo "✅ Makefile generated" || echo "❌ Makefile missing"
[ -f VitaNS.nro ] && echo "✅ VitaNS.nro built ($(du -h VitaNS.nro | cut -f1))" || echo "❌ VitaNS.nro not built"
```

## Getting Additional Help

If you're still experiencing issues:

1. **Check the latest documentation**: Documentation is updated frequently
2. **Search existing issues**: [GitHub Issues](https://github.com/your-repo/VitaNS/issues)
3. **Create a detailed issue report**: Include error messages, environment details, and steps to reproduce
4. **Join the community**: Discord or other community channels
5. **Enable verbose logging**: Use debug builds and detailed error output

### Creating a Good Bug Report

Include:
- **Operating System**: Windows 10/11, Ubuntu version, etc.
- **DevkitPro Version**: `dkp-pacman -Q devkitA64`
- **Mesa Build Hash**: `git rev-parse HEAD` in Mesa directory
- **VitaNS Commit**: `git rev-parse HEAD` in VitaNS directory
- **Full Error Log**: Complete build output or runtime error
- **Steps to Reproduce**: Exact commands that caused the issue
- **Expected vs Actual**: What should happen vs what actually happens

---

This troubleshooting guide covers the most common issues. Keep it bookmarked for quick reference during development!
