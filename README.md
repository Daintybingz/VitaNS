# VitaNS - Nintendo Switch Homebrew Build

## Project Overview
VitaNS is an ambitious homebrew project to port the open-source [Vita3K](https://github.com/Vita3K/Vita3K) PlayStation Vita emulator to the Nintendo Switch. The goal is to enable Switch users to run PS Vita games and homebrew, leveraging the Switch's hardware and homebrew ecosystem. This project is a major technical undertaking, involving deep changes to the emulator core, UI, input, and system integration.

---

## Development Timeline
- **Early 2024:** Project inception, research, and initial feasibility studies.
- **Spring 2024:** Set up devkitPro toolchain, basic Switch homebrew skeleton, and initial code import from Vita3K.
- **Summer 2024:**
  - Ported core emulator modules to build with devkitA64.
  - Replaced platform-specific code (file I/O, threading, input, rendering) with Switch-compatible implementations.
  - Integrated SDL2 for graphics/input and ImGui for UI.
  - Implemented firmware installation from PUP files.
  - Achieved first successful boot of a homebrew Vita app on Switch.
- **Mid 2024:**
  - Refined build system (CMake, Docker, GitHub Actions CI).
  - Fixed multiple definition/linking errors, platform-specific bugs.
  - Added artifact upload to CI for easy .nro download.
  - Improved documentation and developer onboarding.

See [PORTING_PLAN.md](Vita3K-Switch/docs/PORTING_PLAN.md) and [COMPLETION_STATUS.md](Vita3K-Switch/docs/COMPLETION_STATUS.md) for more details.

---

## Major Milestones
- ✅ Core emulator compiles and runs on Switch
- ✅ ImGui-based UI functional on Switch
- ✅ Firmware installation from official PUP files
- ✅ Game loading and basic input working
- ⬜ Audio support (in progress)
- ⬜ Improved game compatibility and performance
- ⬜ Save states, advanced features

---

## Technical Challenges & Solutions
- **Toolchain Differences:**
  - Switched from standard desktop toolchains to devkitPro/devkitA64 for Switch.
  - Replaced unsupported system calls (e.g., `waitpid`, `execvp`) with Switch-safe stubs or guards.
- **Graphics & Input:**
  - Rewrote rendering backend to use SDL2 and GLESv2 for Switch.
  - Integrated ImGui for a modern, portable UI.
- **Filesystem & Save Data:**
  - Adapted file paths and save data handling for Switch SD card layout.
- **Build System:**
  - Unified CMake build, added Docker support for reproducible builds.
  - Set up GitHub Actions for CI, including artifact upload of .nro.
- **Firmware Handling:**
  - Ported and adapted PUP extraction and firmware mounting for Switch filesystem.
- **Debugging:**
  - Used extensive logging (spdlog) and debug builds to trace issues.

See [TECHNICAL_DESIGN.md](Vita3K-Switch/docs/TECHNICAL_DESIGN.md) for in-depth architecture notes.

---

## Porting Process from Vita3K to Switch
- **Initial Import:**
  - Started with a clean fork of Vita3K, stripped out platform-specific code.
- **Incremental Refactoring:**
  - Ported modules one by one, replacing incompatible code with Switch-friendly alternatives.
- **UI & Input:**
  - Replaced desktop UI with ImGui/SDL2.
  - Mapped Switch controls to Vita controls (see [UI_OVERVIEW.md](Vita3K-Switch/docs/UI_OVERVIEW.md)).
- **Testing:**
  - Used homebrew and commercial games to test compatibility.
- **Continuous Integration:**
  - Automated builds and artifact uploads for rapid iteration.

---

## Current Status
- **What Works:**
  - Emulator boots and runs on Switch
  - ImGui UI is fully functional
  - Firmware can be installed from PUP files
  - Many homebrew and some commercial games load and run
  - Input and basic graphics are stable
- **What Doesn't (Yet):**
  - Audio support is incomplete
  - Some games have graphical glitches or performance issues
  - Save states and advanced emulator features are WIP
  - Not all Vita system modules are fully supported

See [COMPLETION_STATUS.md](Vita3K-Switch/docs/COMPLETION_STATUS.md) and [TODO.md](Vita3K-Switch/docs/TODO.md) for up-to-date status.

---

## How to Contribute
- **Read the [BUILD.md](Vita3K-Switch/docs/BUILD.md) for setup instructions.**
- Fork the repo and create feature branches for your changes.
- Submit pull requests with clear descriptions and testing notes.
- Join the [Vita3K Discord](https://discord.gg/vita3k) and devkitPro forums for discussion.
- See [PROJECT_STRUCTURE.md](Vita3K-Switch/docs/PROJECT_STRUCTURE.md) for codebase layout.

---

## Future Plans
- Complete audio support and improve performance
- Expand game compatibility and fix known issues
- Add save states, cheats, and advanced emulator features
- Polish UI/UX for end users
- Upstream Switch-specific improvements to Vita3K where possible

---

## Acknowledgments & Credits
- **Vita3K Team:** For the original emulator and ongoing upstream development
- **devkitPro:** For the Switch homebrew toolchain
- **SDL2, ImGui, spdlog:** For essential libraries
- **Nintendo Switch & PlayStation Vita homebrew communities**
- **All contributors, testers, and users!**

---

For more technical details, see the [docs](Vita3K-Switch/docs/) folder. 