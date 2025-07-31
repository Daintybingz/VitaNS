/*
 * Simplified EGL stub for Nintendo Switch
 * Provides basic EGL symbols for linking
 */

#include <stddef.h>

/* Basic EGL stub functions */
void egl_stub_init(void) {
    /* Empty stub */
}

void egl_stub_cleanup(void) {
    /* Empty stub */
}

/* Export some basic EGL symbols */
int egl_stub_version = 1;
