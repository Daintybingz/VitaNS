/*
 * Minimal Mesa stub for Nintendo Switch
 * This provides basic symbols for linking without the full Mesa implementation
 */

#include <stddef.h>

/* Basic stub functions to satisfy linking */
void mesa_stub_init(void) {
    /* Empty stub */
}

void mesa_stub_cleanup(void) {
    /* Empty stub */
}

/* Export some basic symbols that might be needed */
int mesa_stub_version = 1; 