#pragma once

struct FuseBox {
    // Feature toggles (default: all enabled)
    static bool enableRenderer;
    static bool enableAudio;
    static bool enableInput;
    static bool enableFileSystem;
    static bool enableLogging;
    static bool enableFirmware;
    static bool enableGameLoading;
    static bool enableAboutPage;
    // Add more toggles as needed
}; 