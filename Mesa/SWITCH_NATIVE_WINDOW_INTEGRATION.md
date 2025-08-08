# Switch Native Window Integration Guide

This driver uses a simple native window contract to present frames on Switch without GBM/libdrm/dlopen.

1) Include the header
- Add `#include "switch_native_window.h"` to your app.
- The header is packaged in `mesa-libraries-for-windows/include/`.

2) Implement the window callbacks
- Option A (preferred): provide a `present` callback that copies the provided RGBA8888 buffer to the screen.
- Option B: provide an `acquire_framebuffer` callback that returns a writable framebuffer pointer and stride; the driver will copy into it.

3) Create the window and EGL surface
- Fill out `switch_native_window` (width, height, stride_bytes, format, callbacks).
- Pass it as the native window to `eglCreateWindowSurface`.

4) Swap
- After making the context current, call `eglSwapBuffers`. The driver currently fills a solid color buffer and presents it. This verifies the present path. Rendering output will replace this in the next phase.

Notes
- Pixel format is RGBA8888 for now. Adjust if your display path needs BGRA or RGB565 and perform conversion in your `present` routine.
- If both `present` and `acquire_framebuffer` are provided, the driver prefers `present`.

See `examples/switch_native_window_example.c` for a minimal usage pattern.
