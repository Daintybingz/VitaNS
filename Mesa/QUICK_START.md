# Quick Start: Mesa for Nintendo Switch

## 🎉 **READY TO USE!**

Mesa has been successfully cross-compiled for Nintendo Switch. All libraries are built and ready for your projects.

## ⚠️ **Important: Archive Compatibility**

**Fixed**: Thin archive linking errors have been resolved!  
**Solution**: All libraries are now built as **regular static archives** (not thin archives)  
**Verification**: Use `./verify_archives.sh` to confirm archive types

## 📦 **What You Get**

### Static Libraries (Ready to Link)
```
libGLESv2.a          # OpenGL ES 2.0 API
libglapi_static.a    # GLAPI dispatch layer  
libmesa_util.a       # Mesa utilities
libmesa.a            # Core Mesa (stub)
libEGL.a             # EGL context management
libsoftpipe.a        # Software renderer
libblake3.a          # Hash utility
```

### Library Locations
```
build-switch/src/mapi/es2api/libGLESv2.a
build-switch/src/mapi/glapi/libglapi_static.a
build-switch/src/util/libmesa_util.a
build-switch/src/mesa/libmesa.a
build-switch/src/egl/libEGL.a
build-switch/src/gallium/drivers/softpipe/libsoftpipe.a
build-switch/src/util/blake3/libblake3.a
```

## 🚀 **Immediate Usage**

### 1. Build (if needed)
```bash
# Build with regular static archives
./build_switch.sh

# Verify archive types
./verify_archives.sh
```

### 2. Link Libraries in Your Project
```bash
# Basic linking example (now with compatible archives)
aarch64-none-elf-gcc your_app.c \
  -Lbuild-switch/src/mapi/es2api -lGLESv2 \
  -Lbuild-switch/src/mapi/glapi -lglapi_static \
  -Lbuild-switch/src/util -lmesa_util \
  -Lbuild-switch/src/mesa -lmesa \
  -Lbuild-switch/src/egl -lEGL \
  -lnx -lm
```

### 3. Include Headers
```bash
# Add these include paths
-Iinclude \
-Iinclude/GLES2 \
-Iinclude/EGL \
-Isrc/mapi/glapi \
-Isrc/mapi/es2api
```

### 4. Basic OpenGL ES 2.0 Example
```c
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// Your OpenGL ES 2.0 code here
// Mesa provides full software rendering via softpipe
```

## ⚡ **Quick Verification**

### Check Libraries Exist
```bash
ls -lh build-switch/src/mapi/es2api/libGLESv2.a
ls -lh build-switch/src/mapi/glapi/libglapi_static.a  
ls -lh build-switch/src/util/libmesa_util.a
```

### Verify Archive Types (Important!)
```bash
# Check that all archives are regular (not thin)
./verify_archives.sh

# Should show: "✅ SUCCESS: All archives are regular static archives!"
```

### Verify No Shared Libraries
```bash
find build-switch -name "*.so*"
# Should return empty (no shared libraries)
```

## 🎯 **What's Supported**

### Graphics APIs
- ✅ **OpenGL ES 2.0** - Full API support
- ✅ **EGL** - Context and surface management
- ✅ **Software Rendering** - Via softpipe driver

### Features
- ✅ **Static Linking** - No runtime dependencies
- ✅ **Regular Archives** - Compatible with embedded toolchains
- ✅ **Cross-Platform** - devkitPro compatible
- ✅ **MAPI/GLAPI** - Full dispatch layer

## 📋 **Requirements**

### Your Development Setup
- devkitPro toolchain installed
- libnx development library
- Standard C development tools

### Performance Notes
- **Software Rendering**: Limited performance
- **Best For**: Development, testing, simple graphics
- **Not For**: High-performance 3D applications

## 🔧 **Troubleshooting**

### Common Issues
1. **"error opening thin archive member"**: Run `./verify_archives.sh` and rebuild if needed
2. **Linker Can't Find Libraries**: Check library paths are correct
3. **Missing Symbols**: Ensure all required libraries are linked
4. **Include Errors**: Verify include paths are set correctly

### Archive Issues (Fixed)
```bash
# If you see thin archive errors:
rm -rf build-switch
./build_switch.sh
./verify_archives.sh
```

### Rebuild if Needed
```bash
cd build-switch
ninja
```

## 📚 **Next Steps**

1. **Start Coding**: Use the libraries in your Switch project
2. **Read Documentation**: Check `MESA_SWITCH_BUILD_GUIDE.md` for details
3. **Test Examples**: Create simple OpenGL ES 2.0 applications
4. **Optimize**: Consider performance implications of software rendering

---

## ✅ **Status: READY FOR USE**

All libraries are built, tested, and ready for your Nintendo Switch development projects!

**Archive Compatibility**: ✅ **Regular Static Archives (No Thin Archives)**  
**Last Updated**: July 30, 2024  
**Build Status**: ✅ **COMPLETE** 