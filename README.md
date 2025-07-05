# VitaNS (Vita3K for Nintendo Switch)

VitaNS is a **Nintendo Switch port** of the [Vita3K](https://github.com/Vita3K/Vita3K) PlayStation Vita emulator. This project is based on the excellent work of the original Vita3K development team, who created the cross-platform PlayStation Vita emulator that runs on Windows, macOS, and Linux.

## Acknowledgments

**This project would not be possible without the original Vita3K team.** VitaNS is a direct port of their work, adapted specifically for the Nintendo Switch homebrew platform. We extend our deepest gratitude to:

- **The Vita3K Development Team** - For creating and maintaining the original Vita3K emulator
- **All Vita3K Contributors** - For their ongoing work on PlayStation Vita emulation
- **The Vita3K Community** - For testing, reporting issues, and supporting the project

**Original Vita3K Repository:** https://github.com/Vita3K/Vita3K

## About This Port

VitaNS takes the existing Vita3K codebase and adapts it specifically for Nintendo Switch homebrew, enabling PS Vita games to run on Switch hardware. This is a community effort to bring Vita3K's capabilities to the Switch platform.

## Key Differences from Original Vita3K

VitaNS is **not a 1:1 copy** of Vita3K - it's a significant adaptation for the Nintendo Switch platform:

### **Platform Architecture**
- **Original Vita3K:** Cross-platform (Windows, macOS, Linux)
- **VitaNS:** Nintendo Switch homebrew only

### **Core Adaptations**
- **Switch Platform Layer:** Complete platform abstraction for Switch hardware
- **Input System Overhaul:** Joy-Con button mapping, touch screen support, rear touch panel emulation
- **Memory Management:** Optimized for Switch's 4GB RAM vs Vita's 512MB
- **Display System:** Adapts Vita's 960×544 to Switch's 1280×720 (handheld) / 1920×1080 (docked)
- **Audio Integration:** Switch-specific audio port mapping and buffering
- **File System:** Switch SD card paths and Vita path virtualization
- **UI Framework:** ImGui instead of Plutonium for touch-friendly Switch interface

### **Performance Optimizations**
- **Switch CPU Backend:** ARM-to-ARM optimization opportunities
- **Switch-specific performance profiles** (power saving, balanced, performance)
- **Memory access optimization** for Switch hardware

### **What Remains the Same**
- **Core Vita3K Emulation Logic** - Fundamental PS Vita emulation
- **Module System** - SceCtrl, SceDisplay, SceAudio, etc.
- **Firmware Handling** - PS Vita firmware module loading
- **Game Compatibility** - Maintains compatibility with original Vita3K

## Project Highlights

- **Based on Vita3K:** Direct port of the original Vita3K emulator codebase
- **Cross-platform UI:** Uses [Dear ImGui](https://github.com/ocornut/imgui) for all user interface elements. Plutonium is no longer used.
- **Switch Homebrew:** Targets Nintendo Switch only; not intended for desktop or other platforms.
- **Firmware Handling:** Requires official PS Vita firmware modules (`.suprx` files) extracted from a `.PUP` file. The emulator scans and loads these modules at runtime from the SD card.
- **Linux VM Recommended:** For building the project, a Linux VM with the devkitPro toolchain is recommended to avoid macOS-specific issues.

## System Requirements

### **For Building:**
- Linux VM or Linux system (recommended)
- devkitPro toolchain
- CMake 3.20+
- Git

### **For Running:**
- Nintendo Switch with custom firmware (Atmosphere, SXOS, etc.)
- SD card with at least 4GB free space
- PS Vita firmware files (extracted from official .PUP)

## Control Mapping

### **Default Button Layout:**
- **PS Vita Cross** → Switch A
- **PS Vita Circle** → Switch B  
- **PS Vita Square** → Switch Y
- **PS Vita Triangle** → Switch X
- **PS Vita L1/R1** → Switch L/R
- **PS Vita Start/Select** → Switch Plus/Minus
- **PS Vita D-Pad** → Switch D-Pad
- **PS Vita Front Touch** → Switch Touch Screen
- **PS Vita Rear Touch** → Switch Right Stick (emulated)

*Note: Button mappings can be customized in the settings menu.*

## Installation Guide

### **Prerequisites:**
1. **Custom Firmware:** Ensure your Switch has custom firmware installed
2. **SD Card:** Prepare an SD card with homebrew launcher access
3. **Firmware Files:** Extract PS Vita firmware from official .PUP file

### **Installation Steps:**
1. Download the latest VitaNS.nro from [Releases](../../releases)
2. Create directory: `/switch/VitaNS/` on your SD card
3. Copy VitaNS.nro to `/switch/VitaNS/`
4. Extract PS Vita firmware files to `/switch/VitaNS/firmware/`
5. Launch from homebrew menu

### **Directory Structure:**
```
/switch/VitaNS/
├── VitaNS.nro
├── firmware/
│   ├── *.suprx files
│   └── other firmware modules
├── games/
│   └── your PS Vita games (.vpk files)
└── savedata/
    └── game save data
```

## Development Status

### **Current Status:** 🚧 **In Development**
- **Core Emulation:** ✅ Working
- **Input System:** ✅ Implemented
- **Graphics Rendering:** ✅ Basic support
- **Audio System:** ✅ Basic support
- **UI Framework:** ✅ ImGui integrated
- **Performance Optimization:** 🔄 In progress
- **Game Testing:** 🔄 Ongoing

### **Known Issues:**
- Some games may have performance issues
- Rear touch panel emulation is experimental
- Audio may have occasional stuttering

### **Roadmap:**
- [ ] Performance optimizations
- [ ] Enhanced audio support
- [ ] Additional input customization
- [ ] Game-specific compatibility patches

## Quick Start (For Developers)

1. **Set up a Linux VM** (if not already on Linux).
2. **Install devkitPro** and required Switch libraries:
   ```bash
   wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.deb
   sudo apt install ./devkitpro-pacman.deb
   sudo dkp-pacman -S switch-dev switch-sdl2 switch-mesa switch-glm switch-sdl2_ttf switch-sdl2_image
   ```
3. **Clone the repository:**
   ```bash
   git clone --recursive https://github.com/Daintybingz/VitaNS.git
   cd VitaNS
   ```
4. **Configure and build:**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
   make
   ```
5. **Copy the resulting `VitaNS.nro` to your Switch SD card** under `/switch/VitaNS/`.
6. **Extract PS Vita firmware modules** from an official `.PUP` file and place them on your SD card as described in the user guide.
7. **Launch VitaNS** from the Switch homebrew menu.

## Game Compatibility

VitaNS maintains compatibility with the original Vita3K project. For the latest compatibility information, please refer to:

- **[Vita3K Compatibility Database](https://vita3k.org/compatibility.html)**
- **[Vita3K GitHub Issues](https://github.com/Vita3K/Vita3K/issues)**

### **Performance Notes:**
- Games that run well on Vita3K should perform similarly on VitaNS
- Switch's more powerful hardware may provide better performance in some cases
- Docked mode offers higher resolution but may impact performance

## Documentation

- [Build Instructions](./Vita3K-Switch/docs/BUILD.md)
- [Project Structure](./Vita3K-Switch/docs/PROJECT_STRUCTURE.md)
- [Technical Design](./Vita3K-Switch/docs/TECHNICAL_DESIGN.md)
- [Porting Plan](./Vita3K-Switch/docs/PORTING_PLAN.md)

## Troubleshooting

- If you encounter missing header or include path errors (e.g., `imgui_impl_sdl.h`, `json.hpp`), ensure all submodules are initialized and CMake include paths are correct.
- For persistent build issues, delete your `build/` directory and re-run CMake.
- Use a Linux VM for Switch builds to avoid macOS-specific toolchain issues.

## Support & Community

### **Getting Help:**
- **GitHub Issues:** Report bugs and request features
- **Discord:** Join the Vita3K community for general support
- **Wiki:** Check the documentation for common solutions

### **Contributing:**
We welcome contributions! Please see our [Contributing Guidelines](./CONTRIBUTING.md) for details.

### **Donations:**
If you find VitaNS useful, consider supporting the original Vita3K project:
- **[Vita3K Patreon](https://www.patreon.com/vita3k)**
- **[Vita3K GitHub Sponsors](https://github.com/sponsors/Vita3K)**

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit your changes: `git commit -am 'Add feature'`
4. Push to the branch: `git push origin feature-name`
5. Submit a pull request

## License

VitaNS is based on Vita3K and follows the same license as the upstream project. Please refer to the original Vita3K repository for the complete license information: https://github.com/Vita3K/Vita3K

**Note:** This project is a derivative work of Vita3K. All original Vita3K code and contributions remain under their respective licenses and copyrights. 