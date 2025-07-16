# VitaNS (Vita3K for Nintendo Switch)

## Renderer Backend

VitaNS uses the **GLES2 backend only** for Nintendo Switch, following the proven approach of other successful emulators like **PPSSPP** and **RetroArch**. This ensures:
- Best performance and hardware acceleration
- Maximum compatibility with Switch homebrew
- A well-supported and community-tested rendering path

The software renderer is **not supported on Switch** at this time due to limitations in the devkitPro portlibs (Mesa/EGL packaging). If/when these limitations are resolved, software rendering may be revisited.

For now, all rendering is done via OpenGL ES 2.0, just like PPSSPP and RetroArch on Switch.

## Build Instructions

(Insert your build instructions here)
