# Switch Mesa Runtime Progress Tracker

## Objective
- Run real games/apps on Nintendo Switch using a custom Mesa build (GLES2 + EGL) with software rendering (softpipe).

## Scope
- No GBM/libdrm/dlopen on Switch. Provide a built-in platform EGL driver that creates contexts/surfaces and presents via libnx.

## Current Status (Aug 8, 2024)
- Build: Working static libraries packaged in `mesa-libraries-for-windows/` (regular archives)
- Symbols: `_eglDriver` defined in `libEGL.a`, `_glapi_get_proc_address` defined in `libglapi_static.a`
- Docs: `README.md`, `REBUILD_RESULTS_SUMMARY.md`, `MESA_SWITCH_FINAL_STATUS.md` updated
- Runtime: Stub EGL driver only (links, but does not render). Games won’t run yet.

## Milestones
### 1) Build foundations [DONE]
- [x] Resolve missing components and thin archives
- [x] Preserve required symbols (no aggressive strip)
- [x] Package and verify libraries

### 2) Real Switch EGL driver (surfaceless + softpipe) [IN PROGRESS]
- [x] Replace stub with real `src/egl/drivers/switch/` driver (minimal)
- [x] Implement Initialize/Terminate/CreateContext/MakeCurrent (minimal)
- [x] Implement CreatePbufferSurface/CreateWindowSurface/DestroySurface (minimal)
- [x] Implement SwapBuffers (CPU blit path, solid-color stub)
- [ ] Wire to Gallium softpipe via state tracker (no dlopen)
- [ ] Meson: link `libEGL.a` with required Gallium/state-tracker objects
- [ ] Triangle sample: eglMakeCurrent + glClear on device

### 3) On-screen present via libnx [PLANNED]
- [ ] Use libnx framebuffer as native window
- [ ] Simple swapchain (double-buffer) and memcpy blit
- [ ] Basic swap interval handling

### 4) Integration with Vita3K-Switch [PLANNED]
- [ ] Link with start-group/end-group ordering
- [ ] Boot minimal content; verify frames drawn
- [ ] CI job to run triangle/basic draw

### 5) Performance and compatibility [PLANNED]
- [ ] Threading/tiling tuning for softpipe
- [ ] NEON-tuned paths where available
- [ ] Investigate llvmpipe feasibility for devkitA64
- [ ] Basic profiling and hotspots

## Risks and Constraints
- No GBM/libdrm/dlopen → avoid DRI2 loader path
- CPU-only rendering (softpipe) → games may run slowly
- State tracker wiring is non-trivial; use Haiku/WGL as references

## Decision Log
- Use built-in Switch EGL driver (no loader) to control init/present
- Deliver regular static archives and enforce linker grouping

## Verification Commands
- `_eglDriver` definition:
  nm mesa-libraries-for-windows/libEGL.a | grep -E "\b_eglDriver\b"
- `_glapi_get_proc_address`:
  nm mesa-libraries-for-windows/libglapi_static.a | grep _glapi_get_proc_address
- Linker grouping (example):
  -Wl,--start-group libEGL.a libGLESv2.a libmesa_util.a libsoftpipe.a libblake3.a libmesa.a libglapi_static.a -Wl,--end-group

## Next Actions
- [ ] Wire softpipe/state-tracker context creation so rendering fills surface buffer
- [ ] Replace solid-color fill with rendered pixels
- [ ] Minimal triangle sample to validate on-device present

## References
- `src/egl/drivers/haiku/` and `src/egl/drivers/wgl/` for platform driver patterns
- `README.md`, `REBUILD_RESULTS_SUMMARY.md`, `MESA_SWITCH_FINAL_STATUS.md`
