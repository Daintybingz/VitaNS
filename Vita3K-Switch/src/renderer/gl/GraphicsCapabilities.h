#pragma once
#include <string>

struct GraphicsCapabilities {
    std::string glVersion;
    std::string glRenderer;
    std::string glVendor;
    std::string glExtensions;
    bool has_ES3 = false;
    bool has_MRT = false;
    bool has_UBO = false;
    bool has_instancing = false;
    // Add more as needed
}; 