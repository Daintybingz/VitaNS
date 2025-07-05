#include "SceCtrl.h"
#include "../module_registry.h"
#include <cstdio>
#include <cstring>

// Initialize static members
uint32_t SceCtrl::samplingMode = SCE_CTRL_MODE_DIGITAL;
SceCtrlData SceCtrl::currentState = {};
std::unique_ptr<SwitchInput> SceCtrl::switchInput = nullptr;

SceCtrl::SceCtrl() : Module("SceCtrl") {
    // Initialize controller state
    memset(&currentState, 0, sizeof(SceCtrlData));
    currentState.lx = 128; // Center position
    currentState.ly = 128; // Center position
    currentState.rx = 128; // Center position
    currentState.ry = 128; // Center position
    
    // Initialize Switch input system
    if (!switchInput) {
        switchInput = std::make_unique<SwitchInput>();
        if (!switchInput->initialize()) {
            printf("[SceCtrl] Failed to initialize Switch input system\n");
        } else {
            printf("[SceCtrl] Switch input system initialized successfully\n");
        }
    }
}

SceCtrl::~SceCtrl() {
    // Clean up Switch input system
    if (switchInput) {
        switchInput->finalize();
        switchInput.reset();
        printf("[SceCtrl] Switch input system finalized\n");
    }
}

void SceCtrl::registerFunctions() {
    registerFunction("sceCtrlPeekBufferPositive", 0x9B96A1AA,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port, pad_data (ptr), count
            if (args.size() < 3) return -1;
            return SceCtrl::sceCtrlPeekBufferPositive(args[0], reinterpret_cast<SceCtrlData*>(args[1]), args[2]);
        },
        3
    );
    registerFunction("sceCtrlReadBufferPositive", 0x67E7AB83,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port, pad_data (ptr), count
            if (args.size() < 3) return -1;
            return SceCtrl::sceCtrlReadBufferPositive(args[0], reinterpret_cast<SceCtrlData*>(args[1]), args[2]);
        },
        3
    );
    registerFunction("sceCtrlSetSamplingMode", 0xA497B150,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: mode
            if (args.size() < 1) return -1;
            return SceCtrl::sceCtrlSetSamplingMode(args[0]);
        },
        1
    );
    registerFunction("sceCtrlGetSamplingMode", 0xEC752AAF,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: pMode (ptr)
            if (args.size() < 1) return -1;
            return SceCtrl::sceCtrlGetSamplingMode(reinterpret_cast<uint32_t*>(args[0]));
        },
        1
    );
}

uint32_t SceCtrl::mapSwitchButtonsToVita(uint32_t switchButtons) {
    uint32_t vitaButtons = 0;
    
    // Map Switch buttons to PS Vita buttons
    if (switchButtons & SWITCH_BUTTON_A)      vitaButtons |= SCE_CTRL_CROSS;
    if (switchButtons & SWITCH_BUTTON_B)      vitaButtons |= SCE_CTRL_CIRCLE;
    if (switchButtons & SWITCH_BUTTON_X)      vitaButtons |= SCE_CTRL_TRIANGLE;
    if (switchButtons & SWITCH_BUTTON_Y)      vitaButtons |= SCE_CTRL_SQUARE;
    if (switchButtons & SWITCH_BUTTON_PLUS)   vitaButtons |= SCE_CTRL_START;
    if (switchButtons & SWITCH_BUTTON_MINUS)  vitaButtons |= SCE_CTRL_SELECT;
    if (switchButtons & SWITCH_BUTTON_L)      vitaButtons |= SCE_CTRL_L1;
    if (switchButtons & SWITCH_BUTTON_R)      vitaButtons |= SCE_CTRL_R1;
    if (switchButtons & SWITCH_BUTTON_ZL)     vitaButtons |= SCE_CTRL_L2;
    if (switchButtons & SWITCH_BUTTON_ZR)     vitaButtons |= SCE_CTRL_R2;
    if (switchButtons & SWITCH_BUTTON_LSTICK) vitaButtons |= SCE_CTRL_L3;
    if (switchButtons & SWITCH_BUTTON_RSTICK) vitaButtons |= SCE_CTRL_R3;
    if (switchButtons & SWITCH_BUTTON_UP)     vitaButtons |= SCE_CTRL_UP;
    if (switchButtons & SWITCH_BUTTON_RIGHT)  vitaButtons |= SCE_CTRL_RIGHT;
    if (switchButtons & SWITCH_BUTTON_DOWN)   vitaButtons |= SCE_CTRL_DOWN;
    if (switchButtons & SWITCH_BUTTON_LEFT)   vitaButtons |= SCE_CTRL_LEFT;
    
    return vitaButtons;
}

void SceCtrl::updateControllerState() {
    if (!switchInput) {
        return;
    }
    
    // Update Switch input state
    switchInput->update();
    
    // Get the current controller state
    const SwitchControllerState& switchState = switchInput->getControllerState();
    
    // Update timestamp (would be more accurate in a real implementation)
    currentState.timeStamp++;
    
    // Map buttons
    currentState.buttons = mapSwitchButtonsToVita(switchState.buttons);
    
    // Map analog sticks
    currentState.lx = switchInput->getLeftStickXByte();
    currentState.ly = switchInput->getLeftStickYByte();
    currentState.rx = switchInput->getRightStickXByte();
    currentState.ry = switchInput->getRightStickYByte();
}

int SceCtrl::sceCtrlPeekBufferPositive(uint32_t port, SceCtrlData* pad_data, uint32_t count) {
    printf("[SceCtrl] sceCtrlPeekBufferPositive called: port=%u, count=%u\n", port, count);
    
    if (!pad_data) {
        printf("[SceCtrl] Error: pad_data is NULL\n");
        return -1;
    }
    
    if (port >= 2) {
        printf("[SceCtrl] Error: Invalid port %u\n", port);
        return -1;
    }
    
    // Update controller state
    updateControllerState();
    
    // Copy data to output buffer
    for (uint32_t i = 0; i < count; i++) {
        memcpy(&pad_data[i], &currentState, sizeof(SceCtrlData));
    }
    
    return count;
}

int SceCtrl::sceCtrlReadBufferPositive(uint32_t port, SceCtrlData* pad_data, uint32_t count) {
    printf("[SceCtrl] sceCtrlReadBufferPositive called: port=%u, count=%u\n", port, count);
    
    // This is similar to peek, but should block until new data is available
    // For our stub implementation, we'll just call peek
    return sceCtrlPeekBufferPositive(port, pad_data, count);
}

int SceCtrl::sceCtrlSetSamplingMode(uint32_t mode) {
    printf("[SceCtrl] sceCtrlSetSamplingMode called: mode=%u\n", mode);
    
    if (mode != SCE_CTRL_MODE_DIGITAL && mode != SCE_CTRL_MODE_ANALOG) {
        printf("[SceCtrl] Error: Invalid sampling mode %u\n", mode);
        return -1;
    }
    
    uint32_t oldMode = samplingMode;
    samplingMode = mode;
    
    return oldMode;
}

int SceCtrl::sceCtrlGetSamplingMode(uint32_t* pMode) {
    printf("[SceCtrl] sceCtrlGetSamplingMode called\n");
    
    if (!pMode) {
        printf("[SceCtrl] Error: pMode is NULL\n");
        return -1;
    }
    
    *pMode = samplingMode;
    
    return 0;
}
