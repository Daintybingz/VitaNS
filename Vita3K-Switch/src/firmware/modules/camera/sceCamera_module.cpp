#include "sceCamera.h"
#include "../../../core/module/module_registry.h"

// Register the camera module with the module registry
// REGISTER_MODULE("SceCamera", firmware::modules::camera::sceCameraInit, firmware::modules::camera::sceCameraExit, {
//     // Register functions
//     REG_FUNC(SceCamera, sceCameraOpen);
//     REG_FUNC(SceCamera, sceCameraClose);
//     REG_FUNC(SceCamera, sceCameraStart);
//     REG_FUNC(SceCamera, sceCameraStop);
//     REG_FUNC(SceCamera, sceCameraRead);
//     REG_FUNC(SceCamera, sceCameraSetParameter);
//     REG_FUNC(SceCamera, sceCameraGetParameter);
//     REG_FUNC(SceCamera, sceCameraGetInfo);
//     REG_FUNC(SceCamera, sceCameraGetDeviceInfo);
// });
