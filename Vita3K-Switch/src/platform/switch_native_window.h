#ifndef SWITCH_NATIVE_WINDOW_IMPL_H
#define SWITCH_NATIVE_WINDOW_IMPL_H

#include "switch_native_window.h"

#ifdef __cplusplus
extern "C" {
#endif

// Create a Switch native window with the specified dimensions
// Returns 0 on success, -1 on failure
int switch_native_window_create(switch_native_window *out, int width, int height);

// Destroy a Switch native window and free associated resources
void switch_native_window_destroy(switch_native_window *window);

#ifdef __cplusplus
}
#endif

#endif // SWITCH_NATIVE_WINDOW_IMPL_H
