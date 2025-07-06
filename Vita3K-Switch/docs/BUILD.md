# Building VitaNS (Vita3K for Nintendo Switch)

> **Note:** This project exclusively targets Nintendo Switch homebrew. Building or running on desktop or other platforms is not supported. All instructions and code are Switch-specific and require devkitPro/devkitA64.

> **UI Framework:** VitaNS now uses [Dear ImGui](https://github.com/ocornut/imgui) for its user interface. Plutonium is no longer used due to cross-platform and package availability issues. All UI code and dependencies are ImGui-based.

This document provides instructions for setting up the development environment and building VitaNS for Nintendo Switch.

## Prerequisites

### Hardware Requirements
- Nintendo Switch with custom firmware (for testing)
- Development PC (for building; Linux VM recommended for Switch builds)

### Software Requirements
- [devkitPro](https://devkitpro.org/) with Switch development tools (devkitA64, libnx, switch-sdl2, etc.)
- CMake (version 3.15 or higher)
- Git

## Setting Up the Development Environment

### 1. Install devkitPro (Linux VM Recommended)

#### Linux
1. Install devkitPro pacman:
   ```bash
   wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.deb
   sudo apt install ./devkitpro-pacman.deb
   ```
2. Install Switch development packages:
   ```bash
   sudo dkp-pacman -S switch-dev switch-sdl2 switch-mesa switch-glm switch-sdl2_ttf switch-sdl2_image
```

#### macOS (for code browsing only; Switch builds require Linux)
- You may install devkitPro pacman, but Switch homebrew builds are not supported on macOS. Use a Linux VM for actual builds.

### 2. Clone the Repository

```bash
git clone --recursive https://github.com/yourusername/VitaNS.git
cd VitaNS
```

If you already have the Vita3K source code, you can link it:

```bash
ln -s /path/to/Vita3K-Master ../Vita3K-Master
```

## Building VitaNS

### 1. Configure the Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/cmake/Switch.cmake
```

### 2. Build the Project

```bash
make
```

This will generate a `VitaNS.nro` file that can be run on a Nintendo Switch with custom firmware.

### 3. Running on Switch

1. Copy the `VitaNS.nro` file to your Switch's SD card under `/switch/VitaNS/`
2. Create a `games` directory in the same folder for your PS Vita games
3. Launch the homebrew menu on your Switch and select VitaNS

## Development Workflow

### Building for Development

For development and debugging, you can use:

```bash
make -j$(nproc) # Use multiple cores for faster building
```

### Cleaning the Build

```bash
make clean
```

### Updating the Source

```bash
git pull
git submodule update --init --recursive
```

## Troubleshooting

### Common Issues

1. **CMake can't find devkitPro**
   - Make sure DEVKITPRO environment variable is set
   - On Linux: `echo $DEVKITPRO`

2. **SDL2 or ImGui not found**
   - Verify you installed the Switch SDL2 package: `dkp-pacman -Q switch-sdl2`
   - ImGui is included as a submodule; ensure you used `--recursive` when cloning and run `git submodule update --init --recursive`.

3. **Build fails with missing headers (e.g., imgui_impl_sdl.h, imgui_impl_opengl3.h, json.hpp)**
   - Ensure all submodules are initialized and updated.
   - Check that CMake include paths are set correctly for `external/imgui` and `external/nlohmann`.
   - If you see errors about `#include <json.hpp>`, make sure the include path is set to the directory containing `json.hpp` (not `nlohmann/json.hpp`).

4. **NRO file not launching on Switch**
   - Verify your Switch has the latest Atmosphere/custom firmware
   - Check that the NRO is in the correct location on the SD card

5. **Other Issues**
   - If you encounter persistent build errors, try deleting your `build/` directory and re-running CMake from scratch.
   - For cross-platform development, use a Linux VM for Switch builds to avoid macOS-specific issues.

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit your changes: `git commit -am 'Add feature'`
4. Push to the branch: `git push origin feature-name`
5. Submit a pull request

## Additional Resources

- [devkitPro Documentation](https://devkitpro.org/wiki/Main_Page)
- [libnx Documentation](https://switchbrew.github.io/libnx/index.html)
- [Vita3K Documentation](https://github.com/Vita3K/Vita3K/wiki)
- [Switch Homebrew Development](https://switchbrew.org/wiki/Main_Page)
