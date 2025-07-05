# VitaNS UI Overview

## Current UI Framework: Plutonium
- The main menu and all dialogs are implemented using the Plutonium library for Nintendo Switch homebrew.
- Plutonium provides a native Switch look and feel, with layouts, buttons, file pickers, and toasts.

## Main Menu Structure
- **Install Firmware from PUP**: Opens a file picker for .PUP files, installs firmware.
- **Browse for Game (VPK)**: Opens a file picker for .vpk files in /switch/vitans/games/.
- **Run Selected Game**: Loads and runs the selected VPK using emulator logic.
- **Settings**: Placeholder for future emulator settings (resolution, input, etc.).
- **About / Help**: Shows emulator version, credits, and usage tips.
- **Exit**: Closes the app.

## UI Navigation
- Each menu option is a Plutonium Button.
- File pickers use Plutonium's FileSelector dialog.
- Submenus (Settings, About) are implemented as new Layouts with a Back button.
- Toasts are used for user feedback (success/failure messages).

## Notes for Future Updates
- **Settings**: Add real emulator settings (resolution, input mapping, etc.) to the Settings submenu.
- **Game Management**: Add installed game list, delete, and more advanced game management.
- **Diagnostics**: Add log viewer, system info, and error reporting.
- **Save/Load State**: Add buttons for emulator state management.

## Migration to ImGui (if desired)
- ImGui can replicate all current menu logic: main menu, file pickers, submenus, toasts.
- Each Plutonium Layout/Button can be mapped to an ImGui window/button.
- File pickers would need to be implemented or use an ImGui-compatible file dialog.
- Toasts can be replaced with ImGui popups or notifications.
- Plutonium's navigation stack (layouts) can be mapped to ImGui's window/modal system.

## Version
- This document tracks the UI as of VitaNS v0.1 (Switch port of Vita3K). 