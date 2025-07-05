# VitaNS (Vita3K for Nintendo Switch)

VitaNS is a port of the [Vita3K](https://github.com/Vita3K/Vita3K) PlayStation Vita emulator to the Nintendo Switch homebrew platform. It enables you to run PS Vita games on a Switch with custom firmware.

## Project Highlights

- **Cross-platform UI:** Uses [Dear ImGui](https://github.com/ocornut/imgui) for all user interface elements. Plutonium is no longer used.
- **Switch Homebrew:** Targets Nintendo Switch only; not intended for desktop or other platforms.
- **Firmware Handling:** Requires official PS Vita firmware modules (`.suprx` files) extracted from a `.PUP` file. The emulator scans and loads these modules at runtime from the SD card.
- **Linux VM Recommended:** For building the project, a Linux VM with the devkitPro toolchain is recommended to avoid macOS-specific issues.

## Quick Start

1. **Set up a Linux VM** (if not already on Linux).
2. **Install devkitPro** and required Switch libraries:
   ```bash
   wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.deb
   sudo apt install ./devkitpro-pacman.deb
   sudo dkp-pacman -S switch-dev switch-sdl2 switch-mesa switch-glm switch-sdl2_ttf switch-sdl2_image
   ```
3. **Clone the repository:**
   ```bash
   git clone --recursive https://github.com/yourusername/VitaNS.git
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

## Documentation

- [Build Instructions](./docs/BUILD.md)
- [Project Structure](./docs/PROJECT_STRUCTURE.md)
- [Technical Design](./docs/TECHNICAL_DESIGN.md)
- [Porting Plan](./docs/PORTING_PLAN.md)

## Troubleshooting

- If you encounter missing header or include path errors (e.g., `imgui_impl_sdl.h`, `json.hpp`), ensure all submodules are initialized and CMake include paths are correct.
- For persistent build issues, delete your `build/` directory and re-run CMake.
- Use a Linux VM for Switch builds to avoid macOS-specific toolchain issues.

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit your changes: `git commit -am 'Add feature'`
4. Push to the branch: `git push origin feature-name`
5. Submit a pull request

## License

VitaNS is based on Vita3K and follows the same license as the upstream project. 