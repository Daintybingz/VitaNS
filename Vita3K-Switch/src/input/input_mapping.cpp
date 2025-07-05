#include "input_mapping.h"
#include "switch_input.h"
#include <algorithm>

// Map Vita buttons to Switch buttons
static const std::pair<uint32_t, uint64_t> defaultButtonMappings[] = {
    {SCE_CTRL_CROSS,    SWITCH_BUTTON_A},
    {SCE_CTRL_CIRCLE,   SWITCH_BUTTON_B},
    {SCE_CTRL_SQUARE,   SWITCH_BUTTON_Y},
    {SCE_CTRL_TRIANGLE, SWITCH_BUTTON_X},
    {SCE_CTRL_L1,       SWITCH_BUTTON_L},
    {SCE_CTRL_R1,       SWITCH_BUTTON_R},
    {SCE_CTRL_START,    SWITCH_BUTTON_PLUS},
    {SCE_CTRL_SELECT,   SWITCH_BUTTON_MINUS},
    {SCE_CTRL_UP,       SWITCH_BUTTON_UP},
    {SCE_CTRL_DOWN,     SWITCH_BUTTON_DOWN},
    {SCE_CTRL_LEFT,     SWITCH_BUTTON_LEFT},
    {SCE_CTRL_RIGHT,    SWITCH_BUTTON_RIGHT},
};

void InputMapping::update() {
    if (!switchInput) return; // Ensure SwitchInput is set
    switchInput->update();
    const SwitchControllerState& state = switchInput->getControllerState();

    // Reset current state
    currentVitaButtons = 0;

    // Map Switch buttons to Vita buttons
    for (const auto& mapping : defaultButtonMappings) {
        if (state.buttons & mapping.second) {
            currentVitaButtons |= mapping.first;
        }
    }

    // Normalize stick values to -1.0..1.0
    normalizeStickValues(state.left_stick_x, state.left_stick_y, leftAnalogX, leftAnalogY);
    normalizeStickValues(state.right_stick_x, state.right_stick_y, rightAnalogX, rightAnalogY);
} 