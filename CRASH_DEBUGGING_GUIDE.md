# VitaNS Crash Debugging Guide

## 🔍 SD Card Logging System

VitaNS now includes a comprehensive logging system that writes detailed debug information to your Switch's SD card. This allows us to see exactly where crashes occur, even when the console output isn't visible.

## 📁 Log File Locations

All log files are automatically saved to:
```
sdmc:/switch/vitans/logs/
```

### Log File Types

1. **Main Debug Logs**: `vitans_debug_YYYYMMDD_HHMMSS.log`
   - Complete EGL/Mesa initialization process
   - Step-by-step progress through graphics setup
   - Error messages and crash information

2. **Minimal Test Logs**: `vitans_minimal_YYYYMMDD_HHMMSS.log`
   - Basic Switch functionality testing
   - No EGL/Mesa calls (isolates basic system issues)

## 📊 What the Logs Contain

### System Information
- Build timestamp
- Operation mode (Docked/Handheld)
- Program ID
- Signal handler registration

### Step-by-Step EGL Process
1. **Native Window Creation**
   - Window dimensions and properties
   - Framebuffer initialization
   - Video service setup

2. **EGL Display Setup**
   - Display acquisition
   - EGL version information
   - Vendor and driver details

3. **EGL Configuration**
   - Config selection process
   - Available configurations count

4. **Surface and Context Creation**
   - Surface creation with native window
   - OpenGL ES 2.0 context setup
   - Context activation

5. **OpenGL ES Operations**
   - Clear color setting
   - Frame buffer clearing
   - Buffer swapping

### Crash Detection
- **Signal Handlers**: Catch SIGSEGV, SIGABRT, SIGILL, etc.
- **Last Known Position**: Shows exactly which step completed before crash
- **Error Codes**: EGL and OpenGL error codes with descriptions

## 🔧 How to Use the Logs

### After a Crash:
1. **Remove SD card** from Switch
2. **Navigate to** `switch/vitans/logs/` on your computer
3. **Open the latest log file** (most recent timestamp)
4. **Find the last completed step** before the crash

### Example Log Analysis:

#### Successful Run:
```
=== VitaNS Runtime Debug Version ===
Step 1: Creating native window...
SUCCESS: Native window created: 1280x720
Step 2: Getting EGL display...
SUCCESS: EGL display obtained
Step 3: Initializing EGL...
SUCCESS: EGL initialized: version 1.4
Step 4: Choosing EGL config...
SUCCESS: EGL config chosen
Step 5: Creating EGL surface...
SUCCESS: EGL surface created
...
=== VitaNS Mesa EGL test completed successfully! ===
```

#### Crash During EGL Init:
```
=== VitaNS Runtime Debug Version ===
Step 1: Creating native window...
SUCCESS: Native window created: 1280x720
Step 2: Getting EGL display...
SUCCESS: EGL display obtained
Step 3: Initializing EGL...
[END OF LOG - CRASHED HERE]
```

## 🎯 Debugging Different Crash Points

### Crash at Step 1 (Native Window)
**Likely Issues:**
- Switch video service initialization failure
- Framebuffer creation problems
- Memory allocation issues

**Next Steps:**
- Check if minimal test works
- Verify Switch homebrew environment

### Crash at Step 2 (EGL Display)
**Likely Issues:**
- Mesa platform detection
- EGL driver loading failure
- Missing Mesa components

**Next Steps:**
- Verify Mesa libraries are complete
- Check Mesa build configuration

### Crash at Step 3 (EGL Initialize)
**Likely Issues:**
- Mesa Softpipe driver initialization
- Threading/synchronization problems
- Memory allocation in Mesa internals

**Next Steps:**
- Build Mesa with debug symbols
- Check for missing Mesa dependencies

### Crash at Steps 4-6 (Config/Surface/Context)
**Likely Issues:**
- OpenGL ES configuration problems
- Surface format compatibility
- Context creation failures

**Next Steps:**
- Try different EGL configurations
- Verify OpenGL ES 2.0 support

## 🛠️ Advanced Debugging

### Building with Debug Mesa
If crashes occur in Mesa initialization, build Mesa with debug symbols:
```bash
# In Mesa directory
meson setup build-debug \
    --buildtype=debug \
    -Ddebug=true \
    -Doptimization=0 \
    # ... other Switch-specific options
```

### Enabling Mesa Debug Logging
Add environment variables before calling EGL functions:
```cpp
// In main.cpp, before eglInitialize
setenv("MESA_DEBUG", "1", 1);
setenv("EGL_LOG_LEVEL", "debug", 1);
setenv("LIBGL_DEBUG", "verbose", 1);
```

### Memory Debugging
Add memory allocation tracking:
```cpp
// Check available memory before each major step
u64 freeMem = 0;
svcGetInfo(&freeMem, InfoType_FreeMemorySize, CUR_PROCESS_HANDLE, 0);
DEBUG_LOG("Available memory: %lu bytes", freeMem);
```

## 📋 Reporting Issues

When reporting crashes, please include:

1. **Complete log file** (attach the .log file)
2. **Switch model** (V1/V2/OLED/Lite)
3. **Firmware version** (from crash screen)
4. **Atmosphere version** (from crash screen)
5. **Operation mode** (Docked/Handheld)
6. **Steps to reproduce**

### Log File Template for Issues:
```
**Crash Location:** Step X - [Description]
**Last Successful Step:** [Last completed operation]
**Error Messages:** [Any EGL/GL errors in log]
**System Info:** [Docked/Handheld, firmware version]
**Reproducible:** [Yes/No - happens every time?]
```

## 🎉 Success Indicators

If you see this in the log, VitaNS is working correctly:
```
SUCCESS: All EGL operations completed without crash!
=== VitaNS Mesa EGL test completed successfully! ===
```

This means:
- ✅ Mesa integration is working
- ✅ EGL/OpenGL ES pipeline is functional
- ✅ Switch graphics system is compatible
- ✅ Ready for PS Vita emulation development

---

The SD card logging system provides the visibility needed to debug and fix any remaining runtime issues, bringing us closer to fully functional PS Vita emulation on Nintendo Switch!
