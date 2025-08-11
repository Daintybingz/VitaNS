#ifdef __cplusplus
#error "This must be compiled as C code"
#endif

// Bare metal - no libnx, just raw ARM code
__attribute__((section(".text.main")))
int main(void) {
    // Just loop forever doing nothing
    while(1) {
        __asm__ volatile("wfe"); // Wait for event (low power wait)
    }
    return 0;
}
