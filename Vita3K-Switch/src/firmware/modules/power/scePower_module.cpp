#include "scePower.h"
#include "../../../core/module/module_registry.h"

// Register the power management module with the module registry
REGISTER_MODULE("ScePower", nullptr, nullptr, {
    // Register functions
    REG_FUNC(ScePower, scePowerRegisterCallback);
    REG_FUNC(ScePower, scePowerUnregisterCallback);
    REG_FUNC(ScePower, scePowerGetBatteryLifePercent);
    REG_FUNC(ScePower, scePowerGetBatteryLifeTime);
    REG_FUNC(ScePower, scePowerGetBatteryTemp);
    REG_FUNC(ScePower, scePowerGetBatteryVolt);
    REG_FUNC(ScePower, scePowerIsBatteryCharging);
    REG_FUNC(ScePower, scePowerIsPowerOnline);
    REG_FUNC(ScePower, scePowerGetBatteryStatus);
    REG_FUNC(ScePower, scePowerGetCpuClockFrequency);
    REG_FUNC(ScePower, scePowerSetCpuClockFrequency);
    REG_FUNC(ScePower, scePowerGetBusClockFrequency);
    REG_FUNC(ScePower, scePowerSetBusClockFrequency);
    REG_FUNC(ScePower, scePowerGetGpuClockFrequency);
    REG_FUNC(ScePower, scePowerSetGpuClockFrequency);
    REG_FUNC(ScePower, scePowerRequestColdReset);
    REG_FUNC(ScePower, scePowerRequestStandby);
    REG_FUNC(ScePower, scePowerRequestSuspend);
    REG_FUNC(ScePower, scePowerTick);
});
