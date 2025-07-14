# Renderer Refactor Plan: High-Level, GLES2-Friendly, Backend-Agnostic Design

## Motivation
- Current low-level GXM command buffer replay is fragile and not portable, especially on platforms like Nintendo Switch with limited/broken OpenGL ES support.
- PPSSPP and RetroArch succeed by reconstructing high-level GPU state and using only basic, portable GL features.
- Goal: Build a robust, modular renderer that works everywhere, with software fallback and easy backend extension.

---

## Phased Refactor Roadmap

### **Phase 1: Abstraction Foundation**
**Goal:** Decouple rendering from emulation core and establish a backend-agnostic interface.
- [ ] Define a `Renderer` interface (C++ abstract class) with methods:
    - `init()`
    - `draw_frame(const FrameData&)`
    - `present()`
    - `shutdown()`
- [ ] Add platform/feature detection to select renderer at runtime.
- [ ] Integrate the interface into the emulator core (replace direct GL calls with interface calls).

**Deliverable:** Emulator compiles and runs with a stub (no-op) renderer backend.

---

### **Phase 2: GLES2 Renderer Implementation**
**Goal:** Implement a working renderer using only OpenGL ES 2.0 features.
- [ ] Implement `RendererGLES2` using only OpenGL ES 2.0 features.
- [ ] Dynamically generate GLSL shaders from reconstructed state.
- [ ] Avoid advanced features (MRT, UBO, instancing, etc.).
- [ ] Use SDL2 for window/context creation (no direct EGL usage).

**Deliverable:** Emulator displays graphics using GLES2 backend on PC and Switch (where possible).

---

### **Phase 3: GXM State Reconstruction**
**Goal:** Parse GXM command buffers into high-level GPU state and draw calls.
- [ ] Parse GXM command buffers into high-level state (vertex/index buffers, shaders, textures, blend modes, etc.).
- [ ] Issue draw calls via the Renderer interface, not direct GL calls.

**Deliverable:** Games render correctly using reconstructed state, not low-level command replay.

---

### **Phase 4: Software Renderer Fallback (Optional)**
**Goal:** Provide a software rendering path for platforms with broken or missing GL support.
- [ ] Implement a software rasterizer backend.
- [ ] Upload framebuffer as a texture to a full-screen quad using SDL2.
- [ ] Use as fallback if GL is unavailable/broken.

**Deliverable:** Emulator can run and display output even if OpenGL is not available.

---

### **Phase 5: Platform/Feature Detection and Selection**
**Goal:** Automatically select the best renderer for the platform at runtime.
- [ ] At startup, detect available GL features.
- [ ] Select best renderer implementation for the platform.

**Deliverable:** Emulator chooses the optimal renderer (GLES2 or software) automatically.

---

### **Phase 6: Testing, Validation, and Documentation**
**Goal:** Ensure correctness, performance, and maintainability.
- [ ] Test on Switch, PC, and other GLES2-limited platforms.
- [ ] Validate correctness and performance.
- [ ] Add regression tests for rendering output.
- [ ] Document all interfaces and state translation logic.

**Deliverable:** Stable, portable, and well-documented renderer architecture.

---

## References
- PPSSPP GPU: https://github.com/hrydgard/ppsspp/tree/master/GPU
- RetroArch Video Drivers: https://github.com/libretro/RetroArch/tree/master/gfx/drivers
- SDL2 OpenGL Context: https://wiki.libsdl.org/SDL_GL_CreateContext

---

## Notes
- Start with GLES2 backend, add software fallback next, then consider more advanced backends.
- Keep all new code modular and backend-agnostic.
- Each phase should be completed and reviewed before moving to the next. 