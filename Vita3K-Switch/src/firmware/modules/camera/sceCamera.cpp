#include "sceCamera.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <algorithm>

namespace firmware {
namespace modules {
namespace camera {

// Global camera manager instance
static SceCameraManager g_camera_manager;

// SceCameraManager implementation
SceCameraManager::SceCameraManager()
    : initialized(false) {
}

SceCameraManager::~SceCameraManager() {
    terminate();
}

int SceCameraManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        return SCE_CAMERA_ERROR_ALREADY_INITIALIZED;
    }
    
    printf("[SceCamera] Initializing camera system\n");
    
    // Initialize camera devices
    for (int i = 0; i < SCE_CAMERA_MAX_DEVICES; i++) {
        devices[i] = std::make_unique<CameraDevice>(i);
    }
    
    initialized = true;
    printf("[SceCamera] Camera system initialized successfully\n");
    
    return SCE_CAMERA_ERROR_NONE;
}

int SceCameraManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    printf("[SceCamera] Terminating camera system\n");
    
    // Close all camera devices
    for (auto& device_pair : devices) {
        if (device_pair.second && device_pair.second->is_open()) {
            device_pair.second->close();
        }
    }
    
    // Clear devices
    devices.clear();
    
    initialized = false;
    printf("[SceCamera] Camera system terminated\n");
    
    return SCE_CAMERA_ERROR_NONE;
}

int SceCameraManager::open(int device_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->open();
}

int SceCameraManager::close(int device_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->close();
}

int SceCameraManager::start(int device_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->start();
}

int SceCameraManager::stop(int device_id) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->stop();
}

int SceCameraManager::read(int device_id, SceCameraRead* read) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->read(read);
}

int SceCameraManager::set_parameter(int device_id, int param_type, int value) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->set_parameter(param_type, value);
}

int SceCameraManager::get_parameter(int device_id, int param_type, int* value) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->get_parameter(param_type, value);
}

int SceCameraManager::get_info(int device_id, SceCameraInfo* info) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    return devices[device_id]->get_info(info);
}

int SceCameraManager::get_device_info(SceCameraDeviceInfo* info) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        return SCE_CAMERA_ERROR_NOT_INITIALIZED;
    }
    
    if (!info) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    // Check if the size is valid
    if (info->size < sizeof(SceCameraDeviceInfo)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    // Get device ID from the info structure
    int device_id = info->device_id;
    
    if (!is_valid_device_id(device_id)) {
        return SCE_CAMERA_ERROR_INVALID_PARAM;
    }
    
    // Set device status
    info->status = devices[device_id]->is_open() ? 1 : 0;
    
    return SCE_CAMERA_ERROR_NONE;
}

bool SceCameraManager::is_valid_device_id(int device_id) const {
    return device_id >= 0 && device_id < SCE_CAMERA_MAX_DEVICES;
}

// Module functions

int sceCameraInit() {
    return g_camera_manager.initialize();
}

int sceCameraExit() {
    return g_camera_manager.terminate();
}

int sceCameraOpen(int device_id) {
    return g_camera_manager.open(device_id);
}

int sceCameraClose(int device_id) {
    return g_camera_manager.close(device_id);
}

int sceCameraStart(int device_id) {
    return g_camera_manager.start(device_id);
}

int sceCameraStop(int device_id) {
    return g_camera_manager.stop(device_id);
}

int sceCameraRead(int device_id, SceCameraRead* read) {
    return g_camera_manager.read(device_id, read);
}

int sceCameraSetParameter(int device_id, int param_type, int value) {
    return g_camera_manager.set_parameter(device_id, param_type, value);
}

int sceCameraGetParameter(int device_id, int param_type, int* value) {
    return g_camera_manager.get_parameter(device_id, param_type, value);
}

int sceCameraGetInfo(int device_id, SceCameraInfo* info) {
    return g_camera_manager.get_info(device_id, info);
}

int sceCameraGetDeviceInfo(SceCameraDeviceInfo* info) {
    return g_camera_manager.get_device_info(info);
}

} // namespace camera
} // namespace modules
} // namespace firmware
