#ifndef DLFCN_H
#define DLFCN_H

// Stub dlfcn.h for Switch platform
// Switch doesn't support dynamic linking, so these functions are stubs

#ifdef __cplusplus
extern "C" {
#endif

// Define RTLD_* constants that might be expected
#define RTLD_LAZY    0x00001
#define RTLD_NOW     0x00002
#define RTLD_GLOBAL  0x00100
#define RTLD_LOCAL   0x00000
#define RTLD_NODELETE 0x01000
#define RTLD_NOLOAD  0x00004
#define RTLD_DEEPBIND 0x00008

// Stub function declarations
void *dlopen(const char *filename, int flags);
void *dlsym(void *handle, const char *symbol);
int dlclose(void *handle);
char *dlerror(void);

// Stub implementations
static inline void *dlopen(const char *filename, int flags) {
    (void)filename; (void)flags;
    return NULL; // Always fail on Switch
}

static inline void *dlsym(void *handle, const char *symbol) {
    (void)handle; (void)symbol;
    return NULL; // Always fail on Switch
}

static inline int dlclose(void *handle) {
    (void)handle;
    return 0; // Always succeed
}

static inline char *dlerror(void) {
    return (char*)"Dynamic linking not supported on Switch platform";
}

#ifdef __cplusplus
}
#endif

#endif // DLFCN_H
