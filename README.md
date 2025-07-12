# VitaNS - Nintendo Switch Homebrew Build

VitaNS is a Nintendo Switch homebrew port of the Vita3K emulator, allowing you to run PlayStation Vita games on your Switch.

---

## 🙏 Project Origins & Credits

> VitaNS is based on the open-source [Vita3K](https://github.com/Vita3K/Vita3K) PlayStation Vita emulator.  
> This project would not be possible without the incredible work of the Vita3K team and contributors.  
>  
> The VitaNS team has made significant changes and adaptations to port the emulator to the Nintendo Switch platform, including custom UI, input, and hardware integration.  
>  
> **All credit and thanks to the original Vita3K developers and the homebrew community!**

---

## 🚀 Features

- Play many commercial and homebrew PS Vita games on Nintendo Switch
- ImGui-based user interface for easy navigation
- SDL2-based rendering and input
- Firmware installation from official PUP files
- Game compatibility and performance improvements

---

## 🖥️ System Requirements

- Nintendo Switch with Atmosphere or compatible CFW
- SD card with at least 2GB free space
- Official PlayStation Vita firmware PUP file (for system modules)
- [devkitPro](https://devkitpro.org/) toolchain for building from source

---

## 🎮 Control Mapping

| Switch Button | Vita Button |
|---------------|------------|
| A             | X          |
| B             | O          |
| X             | Square     |
| Y             | Triangle   |
| L/R           | L/R        |
| ZL/ZR         | L2/R2      |
| +             | Start      |
| -             | Select     |
| Left Stick    | Left Stick |
| Right Stick   | Right Stick |

---

## 🛠️ Installation Guide

1. **Download the latest VitaNS.nro** from the [GitHub Actions Artifacts](#) (or build from source).
2. Place `VitaNS.nro` on your SD card in `/switch/vitans/`.
3. (Optional) Place your PS Vita firmware PUP file in `/switch/vitans/firmware/`.
4. Launch VitaNS from the Homebrew Menu on your Switch.
5. Follow the on-screen instructions to install firmware and load games.

---

## 🏗️ Building from Source

1. Install [devkitPro](https://devkitpro.org/) and set up the Switch toolchain.
2. Clone this repository:
   ```sh
   git clone https://github.com/Daintybingz/VitaNS.git
   cd VitaNS
   ```
3. Build the project:
   ```sh
   cd Vita3K-Switch
   mkdir build && cd build
   cmake ..
   make
   ```
4. The output `VitaNS.nro` will be in the `build` directory.

---

## 📈 Development Status

- [x] Basic emulator core ported
- [x] ImGui UI working
- [x] Firmware installation
- [ ] Audio support (WIP)
- [ ] Game compatibility improvements
- [ ] Save states and more...

---

## 🕹️ Game Compatibility

- Many homebrew and some commercial games are playable.
- See the [compatibility list](docs/compatibility.md) for details.

---

## 🤝 Support & Community

- [devkitPro Forums](https://devkitpro.org/viewforum.php?f=7)
- [Vita3K Discord](https://discord.gg/vita3k)
- [GitHub Issues](https://github.com/Daintybingz/VitaNS/issues) for bug reports

---

## 📜 License

This project is licensed under the GPLv2. See [COPYING.txt](COPYING.txt) for details.

---

## 👏 Additional Credits

- [Vita3K Team](https://github.com/Vita3K)
- devkitPro
- SDL2, ImGui, and all open-source contributors
