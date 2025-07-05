#pragma once
#include "../module.h"
#include "../../input/switch_input.h"
#include <cstdint>
#include <memory>

// PS Vita controller button masks
#define SCE_CTRL_SELECT      0x00000001
#define SCE_CTRL_L3          0x00000002
#define SCE_CTRL_R3          0x00000004
#define SCE_CTRL_START       0x00000008
#define SCE_CTRL_UP          0x00000010
#define SCE_CTRL_RIGHT       0x00000020
#define SCE_CTRL_DOWN        0x00000040
#define SCE_CTRL_LEFT        0x00000080
#define SCE_CTRL_L2          0x00000100
#define SCE_CTRL_R2          0x00000200
#define SCE_CTRL_L1          0x00000400
#define SCE_CTRL_R1          0x00000800
#define SCE_CTRL_TRIANGLE    0x00001000
#define SCE_CTRL_CIRCLE      0x00002000
#define SCE_CTRL_CROSS       0x00004000
#define SCE_CTRL_SQUARE      0x00008000

// PS Vita controller ports
#define SCE_CTRL_PORT_0      0
#define SCE_CTRL_PORT_1      1

// PS Vita controller mode
#define SCE_CTRL_MODE_DIGITAL    0
#define SCE_CTRL_MODE_ANALOG     1

// PS Vita controller data structure
typedef struct SceCtrlData {
    uint64_t    timeStamp;
    uint32_t    buttons;
    uint8_t     lx;
    uint8_t     ly;
    uint8_t     rx;
    uint8_t     ry;
    uint8_t     reserved[16];
} SceCtrlData;

class SceCtrl : public Module {
public:
    SceCtrl();
    virtual ~SceCtrl();
    void registerFunctions() override;

    // System call implementations
    static int sceCtrlPeekBufferPositive(uint32_t port, SceCtrlData* pad_data, uint32_t count);
    static int sceCtrlReadBufferPositive(uint32_t port, SceCtrlData* pad_data, uint32_t count);
    static int sceCtrlSetSamplingMode(uint32_t mode);
    static int sceCtrlGetSamplingMode(uint32_t* pMode);

private:
    static uint32_t samplingMode;
    static SceCtrlData currentState;
    static std::unique_ptr<SwitchInput> switchInput;
    
    // Helper to update controller state from Switch inputs
    static void updateControllerState();
    
    // Map Switch buttons to PS Vita buttons
    static uint32_t mapSwitchButtonsToVita(uint32_t switchButtons);
};
