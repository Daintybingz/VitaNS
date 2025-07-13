# Vita3K Switch Port: Full Graphics Implementation Pathway

This document outlines the recommended, phased approach for implementing the graphics subsystem of Vita3K on the Nintendo Switch. It is designed to maximize compatibility, maintainability, and performance, while working within the constraints of the Switch homebrew environment.

---

## PHASE 1: Core Graphics Framework

**Goal:** Set up a working rendering system that boots, shows output, and draws basic scenes.

1. **Initialize SDL2 + OpenGL ES (via EGL)**
    - Use SDL2 for window/context/input handling.
    - Request OpenGL ES 2.0/3.0 context.
    - Confirm and log:
        - GL version
        - GL extensions
        - Context success/failure
    - Fall back gracefully if GL ES 3.0 is not available.

2. **Abstract Graphics Interface**
    - Create:
        - `IGraphicsBackend`
        - `RendererGLES2`, `RendererGLES3`
    - Define virtual methods:
        - `CreateShader()`
        - `BindTexture()`
        - `SetFramebuffer()`
        - `Draw()`
    - Select renderer based on detected capabilities.

3. **Basic Framebuffer & Texture System**
    - Implement:
        - FBO creation
        - Texture creation/loading
        - `glTexImage2D` / `glFramebufferTexture2D`
    - Emulate multiple render targets via ping-pong framebuffers if MRT is unsupported.

---

## PHASE 2: Shader Translation + Rendering Pipeline

**Goal:** Convert GXM shaders → GLSL ES and execute accurate draw calls.

4. **GXM Shader Parser**
    - Adapt Vita3K’s shader translator to target GLSL ES 3.0.
    - Inject:
        - `precision mediump float;`
        - `attribute`/`varying` (ES 2.0) or `in`/`out` (ES 3.0)
    - Strip or emulate:
        - `layout(location=...)`
        - `textureLod()`, `dFdx`/`dFdy` (only if supported)

5. **Shader Compilation and Caching**
    - Compile translated shaders.
    - Implement a persistent disk cache:
        - Hash original GXM shader
        - Cache compiled GLSL version + binary if possible
    - On failure, log GXM + GLSL source to SD for debugging.

6. **Implement Draw Call Execution**
    - Translate GXM draw call → GL draw call:
        - Bind program
        - Upload uniforms/textures
        - Set vertex attributes
        - Issue `glDrawArrays` / `glDrawElements`

---

## PHASE 3: GPU Command Buffer and State Management

**Goal:** Parse and execute GPU commands sent by PS Vita games.

7. **GXM Command Buffer Emulation**
    - Parse:
        - Draw calls
        - Clear commands
        - Texture binds
        - Shader swaps
    - Implement command queue system:
        - Validate order of operations
        - Reconstruct draw state per frame

8. **Emulate GPU State Machine**
    - Track internal:
        - Blend state
        - Depth/stencil settings
        - Scissor + viewport
        - Shader bindings
    - Avoid redundant gl* calls — batch intelligently.

---

## PHASE 4: Texture + Format Handling

**Goal:** Accurately display textures as used by real Vita games.

9. **Implement Texture Swizzle/Unsizzle**
    - Write routines to handle:
        - Tiled memory
        - Swizzled formats
        - PVRTC decompression
    - Convert on CPU if needed → upload linear textures to GL.

10. **Format Translation Layer**
    - Map Vita formats to GL ES-compatible formats:
        - RGBA8 → GL_RGBA
        - RGB565 → GL_RGB565
        - DXT/PVRTC → decode or skip with fallback

---

## PHASE 5: Advanced Rendering Support

**Goal:** Increase compatibility with more titles and improve visuals.

11. **Framebuffer Effects**
    - Handle:
        - Offscreen render targets
        - Depth-only passes
        - Post-processing
    - Emulate:
        - Alpha blending modes
        - Depth/stencil masking

12. **Emulate Missing GL ES 3.2 Features**
    - If unsupported:
        - MRT → multiple passes + blending
        - UBO → use textures or struct arrays
        - Instancing → manually loop through instances
        - LogicOps → emulate in shader

---

## PHASE 6: Debugging, Testing, and Performance

**Goal:** Ensure stability, performance, and visual correctness.

13. **On-Screen Debug UI (Optional)**
    - Render:
        - FPS
        - Framebuffer previews
        - Shader errors

14. **Diagnostic Logging System**
    - Log shader compiles, GL errors, unsupported features
    - Allow SD card dump of:
        - Shader source
        - Render pipeline stats
        - Texture data

15. **Frame Profiler (Optional)**
    - Count:
        - Draw calls/frame
        - State changes
        - Shader switches
    - Display using ImGui or simple in-game overlay

---

## PHASE 7: Game Compatibility Layer

**Goal:** Make Vita game content render as it does on original hardware.

16. **Per-Game Render Fixups (If Needed)**
    - Detect title IDs or shaders and apply:
        - Manual tweaks
        - Pre-translated shaders
        - Special framebuffer setups

17. **Save State + Frame Capture (Optional)**
    - Save GL state + textures
    - Replay draw calls for regression testing

---

## Milestone Checklist

| Phase | Milestone                                      | Status |
|-------|------------------------------------------------|--------|
| 1     | SDL2 + OpenGL ES Context with fallback         | ✅     |
| 2     | Renderer abstraction with 2.0/3.0 paths        | ✅     |
| 3     | Shader translator: GXM → GLSL ES               | ⬜     |
| 4     | Texture loader + swizzle/unswizzle             | ⬜     |
| 5     | GPU command execution from Vita3K              | ⬜     |
| 6     | Accurate framebuffer and post-processing       | ⬜     |
| 7     | Debug overlay + logging                        | ⬜     |
| 8     | Run first real game with working graphics      | ⬜     |

---

## Notes & Recommendations
- **Base your renderer on OpenGL ES 2.0, with conditional ES 3.0+ support.**
- **Avoid relying on advanced features** (MRTs, UBOs, instancing, framebuffer fetch) unless detected at runtime.
- **Use a shader translator** that targets `#version 300 es` with ES 2.0 fallbacks.
- **Detect and log GPU capabilities** at runtime (to file or screen).
- **Modularize rendering logic** by GL version for maintainability and future-proofing.

---

*This document should be updated as milestones are completed and new technical challenges are discovered.* 