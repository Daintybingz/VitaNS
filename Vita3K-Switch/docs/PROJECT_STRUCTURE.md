# VitaNS Project Structure

This document outlines the organization of the VitaNS project, which ports the Vita3K PlayStation Vita emulator to Nintendo Switch.

## Directory Structure

```
VitaNS/
├── assets/                  # Images, icons, and other assets
├── build/                   # Build output directory
├── docs/                    # Documentation
│   ├── BUILD.md             # Build instructions
│   ├── PORTING_PLAN.md      # Detailed porting strategy
│   ├── PROJECT_STRUCTURE.md # This file
│   ├── TECHNICAL_DESIGN.md  # Technical implementation details
│   └── USER_GUIDE.md        # End-user documentation (future)
├── src/                     # Source code
│   ├── core/                # Core emulation components
│   │   ├── cpu/             # CPU emulation adaptations
│   │   ├── memory/          # Memory management
│   │   └── timing/          # System timing
│   ├── filesystem/          # File system implementation
│   ├── firmware/            # PS Vita firmware modules (runtime scanning and loading of .suprx modules)
│   │   └── modules/         # PS Vita OS modules
│   │       ├── trophy/      # Trophy system implementation
│   │       ├── camera/      # Camera emulation
│   │       └── power/       # Power management
│   ├── input/               # Input handling and mapping
│   │   ├── input_mapping.h  # PS Vita to Switch input mapping
│   │   └── input_mapping.cpp
│   ├── platform/            # Platform-specific code
│   │   ├── switch_platform.h    # Switch platform abstraction
│   │   └── switch_platform.cpp
│   ├── renderer/            # Graphics rendering
│   │   ├── gl/              # OpenGL ES implementation
│   │   └── shaders/         # Shader adaptations
│   ├── ui/                  # User interface (ImGui-based)
│   │   ├── imgui/           # ImGui implementation
│   │   └── settings/        # Settings UI
│   └── main.cpp             # Application entry point
├── CMakeLists.txt           # CMake build configuration
└── README.md                # Project overview
```

## Component Overview

### Core Components

1. **CPU Emulation**
   - Adapts Vita3K's ARM emulation for Switch
   - Optimizes for ARM-to-ARM translation

2. **Memory Management**
   - Handles PS Vita memory layout virtualization
   - Optimizes memory usage for Switch

3. **File System**
   - Provides access to Switch SD card
   - Maintains PS Vita file system structure

### Platform Layer

The platform layer abstracts Switch-specific functionality:

1. **Switch Platform**
   - Handles Switch hardware detection (docked/handheld)
   - Manages screen dimensions and scaling
   - Provides system information

2. **Input System**
   - Maps Joy-Con controls to PS Vita controls
   - Handles touch input
   - Emulates rear touch panel

### Rendering System

1. **OpenGL ES Renderer**
   - Adapts Vita3K's renderer for Switch
   - Handles shader translation
   - Manages textures and framebuffers

2. **UI System**
   - Provides user interface using ImGui (cross-platform, tool-like UI)
   - Adapts UI for Switch screen
   - Implements touch-friendly controls
   - Plutonium is no longer used; all UI is ImGui-based for portability and easier development

## Build System

The project uses CMake with devkitPro toolchain:

1. **CMakeLists.txt**
   - Configures build for Switch
   - Sets compiler flags and options
   - Manages dependencies

2. **Build Process**
   - Compiles C++ code for Switch
   - Links with required libraries
   - Generates NRO file for Switch homebrew
   - **Linux VM is recommended for Switch builds** to avoid macOS-specific issues

## Firmware Handling

- VitaNS requires PS Vita firmware modules (e.g., `.suprx` files) to be extracted from an official firmware `.PUP` file.
- The emulator scans and loads required modules at runtime from the SD card.
- See the user guide for instructions on extracting and installing firmware modules.

## Integration with Vita3K

The project integrates with the original Vita3K codebase through:

1. **Platform Abstraction**
   - Isolates platform-specific code
   - Allows reuse of core emulation components

2. **Shared Components**
   - Reuses CPU emulation core
   - Adapts rendering pipeline
   - Maintains module implementation
