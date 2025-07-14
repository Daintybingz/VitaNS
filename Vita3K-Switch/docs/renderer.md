# VitaNS Renderer Refactor & GXM Command Flow

## Overview

This document describes the new high-level, backend-agnostic renderer architecture for VitaNS, focusing on the integration of GXM command parsing, buffer and shader management, and extensibility for future features and platforms.

---

## Architecture

- **Renderer Abstraction:**
  - The renderer is now backend-agnostic, with a `Renderer` interface and a concrete `RendererGLES2` implementation for OpenGL ES 2.0.
  - The renderer is integrated with the emulator core and receives commands via a GXM command buffer system.

- **GXM Command Flow:**
  1. **Emulated game issues GXM commands** (draw, set state, upload buffer, etc.).
  2. **SceGxm module** extracts relevant data (vertex/index buffers, shaders) from emulated memory and emits high-level commands.
  3. **GpuSubsystem** parses the command stream and queues commands for execution.
  4. **GxmExecutor** dispatches commands to the renderer backend.
  5. **RendererGLES2** performs actual GL calls, using uploaded buffers and shaders.

---

## Supported GXM Command Types

- `Draw` — Issues a draw call with the current state, buffers, and shader.
- `Clear` — Clears framebuffer with specified mask.
- `BindTexture` — Binds a texture to a unit.
- `BindShader` — Selects a shader by name.
- `SetState` — Sets GPU state (blend, depth, stencil, etc.).
- `UploadVertexBuffer` — Uploads vertex data from emulated memory to GL.
- `UploadIndexBuffer` — Uploads index data from emulated memory to GL.
- `UploadShader` — Uploads a translated GLSL shader for use in rendering.

**Command Binary Layouts:**
- See `GpuSubsystem::parseGxmCommandStream` for details on each command's binary format.

---

## Shader Management

- **Translation:**
  - Vita GXM shaders are translated to GLSL using `SceGxm::translateShader`.
  - Both vertex and fragment shaders are supported.
- **Upload:**
  - Translated shaders are uploaded to the renderer via `UploadShader` commands.
- **Selection:**
  - Shaders can be selected dynamically by name or program ID using `setCurrentShader`/`setCurrentShaderById`.

---

## Buffer Management

- **Vertex Buffers:**
  - Extracted from emulated memory and uploaded via `UploadVertexBuffer`.
  - Bound to GL and used for draw calls.
- **Index Buffers:**
  - Extracted and uploaded via `UploadIndexBuffer`.
  - Used for indexed draw calls.

---

## Vertex Attribute Setup

- The renderer supports flexible attribute setup via `setupVertexAttributes`.
- Attribute layouts can be extended to match real game formats (e.g., position, normal, texcoord, color).
- Example: `{ {0, 3}, {1, 2} }` for position (vec3) and texcoord (vec2).

---

## Extending the System

- **Adding New Commands:**
  - Define a new command struct in `GxmCommandBuffer.h`.
  - Parse and emit the command in `GpuSubsystem` and `SceGxm` as needed.
  - Handle the command in `GxmExecutor` and implement logic in the renderer.

- **Adding New Backends:**
  - Implement the `Renderer` interface for the new backend (e.g., Vulkan, software).
  - Integrate with the command flow as in `RendererGLES2`.

---

## Testing & Debugging Tips

- Use debug prints in `GxmExecutor` and `RendererGLES2` to trace command execution.
- Verify buffer uploads and shader usage by inspecting GL state.
- Add new attribute layouts and test with homebrew or test ROMs.
- Use the modular design to isolate and debug specific renderer or command issues.

---

## Future Work

- Dynamic attribute setup based on real vertex descriptors.
- Full uniform and sampler management for shaders.
- Resource cleanup and error handling for uploaded buffers and shaders.
- Support for additional GXM features and more complex rendering pipelines. 