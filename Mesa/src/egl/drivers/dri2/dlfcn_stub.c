/* Minimal dlfcn.h stubs for Nintendo Switch build (no dynamic loading) */
#include <stddef.h>

#define RTLD_LAZY 0
#define RTLD_NOW 0
#define RTLD_LOCAL 0
#define RTLD_GLOBAL 0

void *dlopen(const char *filename, int flag) {
    (void)filename; (void)flag;
    return NULL;
}

char *dlerror(void) {
    return "dlopen/dlsym unsupported on this platform";
}

void *dlsym(void *handle, const char *symbol) {
    (void)handle; (void)symbol;
    return NULL;
}

int dlclose(void *handle) {
    (void)handle;
    return 0;
}


