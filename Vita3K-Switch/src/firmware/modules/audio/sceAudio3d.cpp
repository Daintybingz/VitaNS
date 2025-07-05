#include "sceAudio3d.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace firmware {
namespace modules {
namespace audio {

// Global 3D audio manager instance
static SceAudio3dManager g_audio3d_manager;

// SceAudio3dManager implementation
SceAudio3dManager::SceAudio3dManager()
    : initialized(false), 
      next_source_handle(1), 
      next_room_handle(1), 
      next_port_handle(1) {
    
    // Initialize listener parameters to default values
    memset(&listener_params, 0, sizeof(SceAudio3dListenerParam));
    listener_params.forward[0] = 0.0f;
    listener_params.forward[1] = 0.0f;
    listener_params.forward[2] = 1.0f;
    listener_params.up[0] = 0.0f;
    listener_params.up[1] = 1.0f;
    listener_params.up[2] = 0.0f;
}

SceAudio3dManager::~SceAudio3dManager() {
    if (initialized) {
        terminate();
    }
}

int SceAudio3dManager::initialize() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (initialized) {
        std::cerr << "3D audio manager is already initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_ALREADY_INITIALIZED;
    }
    
    std::cout << "Initializing 3D audio manager" << std::endl;
    
    // Clear maps
    sources.clear();
    rooms.clear();
    ports.clear();
    
    // Reset next handle values
    next_source_handle = 1;
    next_room_handle = 1;
    next_port_handle = 1;
    
    initialized = true;
    std::cout << "3D audio manager initialized successfully" << std::endl;
    return 0;
}

int SceAudio3dManager::terminate() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    std::cout << "Terminating 3D audio manager" << std::endl;
    
    // Clear maps
    sources.clear();
    rooms.clear();
    ports.clear();
    
    initialized = false;
    std::cout << "3D audio manager terminated successfully" << std::endl;
    return 0;
}

int SceAudio3dManager::create_source(SceAudio3dSourceHandle *handle, const SceAudio3dSourceParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!handle || !params) {
        std::cerr << "Invalid handle or parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Check if we have reached the maximum number of sources
    if (sources.size() >= SCE_AUDIO3D_MAX_SOURCES) {
        std::cerr << "Maximum number of sources reached" << std::endl;
        return SCE_AUDIO3D_ERROR_OUT_OF_MEMORY;
    }
    
    // Create a new source
    SceAudio3dSourceHandle source_handle = next_source_handle++;
    
    Audio3dSourceState source;
    source.handle = source_handle;
    source.params = *params;
    source.active = true;
    source.room_handle = 0; // No room assigned
    
    // Add the source to the map
    sources[source_handle] = source;
    
    *handle = source_handle;
    
    std::cout << "Created 3D audio source: " << source_handle << std::endl;
    return 0;
}

int SceAudio3dManager::release_source(SceAudio3dSourceHandle handle) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_source_handle(handle)) {
        std::cerr << "Invalid source handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    // Remove the source from any rooms it's in
    for (auto &room_pair : rooms) {
        auto &room = room_pair.second;
        auto it = std::find(room.sources.begin(), room.sources.end(), handle);
        if (it != room.sources.end()) {
            room.sources.erase(it);
        }
    }
    
    // Remove the source from the map
    sources.erase(handle);
    
    std::cout << "Released 3D audio source: " << handle << std::endl;
    return 0;
}

int SceAudio3dManager::set_source_params(SceAudio3dSourceHandle handle, const SceAudio3dSourceParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_source_handle(handle)) {
        std::cerr << "Invalid source handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (!params) {
        std::cerr << "Invalid parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Update source parameters
    sources[handle].params = *params;
    
    std::cout << "Set 3D audio source parameters: " << handle << std::endl;
    return 0;
}

int SceAudio3dManager::get_source_params(SceAudio3dSourceHandle handle, SceAudio3dSourceParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_source_handle(handle)) {
        std::cerr << "Invalid source handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (!params) {
        std::cerr << "Invalid parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Get source parameters
    *params = sources[handle].params;
    
    return 0;
}

int SceAudio3dManager::set_source_room(SceAudio3dSourceHandle handle, SceAudio3dRoomHandle room_handle) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_source_handle(handle)) {
        std::cerr << "Invalid source handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (room_handle != 0 && !is_valid_room_handle(room_handle)) {
        std::cerr << "Invalid room handle: " << room_handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    // Remove the source from its current room
    SceAudio3dRoomHandle current_room_handle = sources[handle].room_handle;
    if (current_room_handle != 0) {
        auto &current_room = rooms[current_room_handle];
        auto it = std::find(current_room.sources.begin(), current_room.sources.end(), handle);
        if (it != current_room.sources.end()) {
            current_room.sources.erase(it);
        }
    }
    
    // Set the new room
    sources[handle].room_handle = room_handle;
    
    // Add the source to the new room
    if (room_handle != 0) {
        rooms[room_handle].sources.push_back(handle);
    }
    
    std::cout << "Set 3D audio source room: " << handle << " -> " << room_handle << std::endl;
    return 0;
}

int SceAudio3dManager::output_source(SceAudio3dSourceHandle handle, const void *buffer, uint32_t samples) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_source_handle(handle)) {
        std::cerr << "Invalid source handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (!buffer) {
        std::cerr << "Invalid buffer pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    auto &source = sources[handle];
    
    // Calculate buffer size in samples
    uint32_t buffer_size = samples * source.params.channel_count;
    
    // Resize source buffer if needed
    if (source.buffer.size() < buffer_size) {
        source.buffer.resize(buffer_size);
    }
    
    // Copy audio data to source buffer
    const int16_t *src = static_cast<const int16_t*>(buffer);
    
    // Convert to float for processing
    for (uint32_t i = 0; i < buffer_size; i++) {
        source.buffer[i] = src[i] / 32768.0f;
    }
    
    // Apply 3D audio processing
    // In a real implementation, this would apply various 3D audio effects
    // For now, we'll just store the data
    
    return 0;
}

int SceAudio3dManager::set_listener_params(const SceAudio3dListenerParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!params) {
        std::cerr << "Invalid parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Update listener parameters
    listener_params = *params;
    
    std::cout << "Set 3D audio listener parameters" << std::endl;
    return 0;
}

int SceAudio3dManager::get_listener_params(SceAudio3dListenerParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!params) {
        std::cerr << "Invalid parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Get listener parameters
    *params = listener_params;
    
    return 0;
}

int SceAudio3dManager::create_room(SceAudio3dRoomHandle *handle, const SceAudio3dRoomParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!handle || !params) {
        std::cerr << "Invalid handle or parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Check if we have reached the maximum number of rooms
    if (rooms.size() >= SCE_AUDIO3D_MAX_ROOMS) {
        std::cerr << "Maximum number of rooms reached" << std::endl;
        return SCE_AUDIO3D_ERROR_OUT_OF_MEMORY;
    }
    
    // Create a new room
    SceAudio3dRoomHandle room_handle = next_room_handle++;
    
    Audio3dRoomState room;
    room.handle = room_handle;
    room.params = *params;
    room.active = true;
    
    // Add the room to the map
    rooms[room_handle] = room;
    
    *handle = room_handle;
    
    std::cout << "Created 3D audio room: " << room_handle << std::endl;
    return 0;
}

int SceAudio3dManager::release_room(SceAudio3dRoomHandle handle) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_room_handle(handle)) {
        std::cerr << "Invalid room handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    // Remove all sources from this room
    auto &room = rooms[handle];
    for (auto source_handle : room.sources) {
        if (is_valid_source_handle(source_handle)) {
            sources[source_handle].room_handle = 0;
        }
    }
    
    // Remove the room from the map
    rooms.erase(handle);
    
    std::cout << "Released 3D audio room: " << handle << std::endl;
    return 0;
}

int SceAudio3dManager::set_room_params(SceAudio3dRoomHandle handle, const SceAudio3dRoomParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_room_handle(handle)) {
        std::cerr << "Invalid room handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (!params) {
        std::cerr << "Invalid parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Update room parameters
    rooms[handle].params = *params;
    
    std::cout << "Set 3D audio room parameters: " << handle << std::endl;
    return 0;
}

int SceAudio3dManager::get_room_params(SceAudio3dRoomHandle handle, SceAudio3dRoomParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_room_handle(handle)) {
        std::cerr << "Invalid room handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (!params) {
        std::cerr << "Invalid parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Get room parameters
    *params = rooms[handle].params;
    
    return 0;
}

int SceAudio3dManager::create_port(SceAudio3dPortHandle *handle, const SceAudio3dPortParam *params) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!handle || !params) {
        std::cerr << "Invalid handle or parameters pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    // Create a new port
    SceAudio3dPortHandle port_handle = next_port_handle++;
    
    Audio3dPortState port;
    port.handle = port_handle;
    port.params = *params;
    port.active = true;
    
    // Initialize buffer
    port.buffer.resize(params->grain * params->channel_count, 0);
    
    // Add the port to the map
    ports[port_handle] = port;
    
    *handle = port_handle;
    
    std::cout << "Created 3D audio port: " << port_handle << std::endl;
    return 0;
}

int SceAudio3dManager::release_port(SceAudio3dPortHandle handle) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_port_handle(handle)) {
        std::cerr << "Invalid port handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    // Remove the port from the map
    ports.erase(handle);
    
    std::cout << "Released 3D audio port: " << handle << std::endl;
    return 0;
}

int SceAudio3dManager::output_port(SceAudio3dPortHandle handle, void *buffer, uint32_t samples) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!initialized) {
        std::cerr << "3D audio manager is not initialized" << std::endl;
        return SCE_AUDIO3D_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_port_handle(handle)) {
        std::cerr << "Invalid port handle: " << handle << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_HANDLE;
    }
    
    if (!buffer) {
        std::cerr << "Invalid buffer pointer" << std::endl;
        return SCE_AUDIO3D_ERROR_INVALID_PARAMETER;
    }
    
    auto &port = ports[handle];
    
    // Calculate buffer size in samples
    uint32_t buffer_size = samples * port.params.channel_count;
    
    // Process 3D audio
    process_audio();
    
    // Copy audio data from port buffer to output buffer
    int16_t *dst = static_cast<int16_t*>(buffer);
    std::copy(port.buffer.begin(), port.buffer.begin() + buffer_size, dst);
    
    // Clear port buffer
    std::fill(port.buffer.begin(), port.buffer.end(), 0);
    
    return 0;
}

void SceAudio3dManager::process_audio() {
    // Process all active sources
    for (auto &source_pair : sources) {
        auto &source = source_pair.second;
        
        // Skip inactive sources
        if (!source.active || source.buffer.empty()) {
            continue;
        }
        
        // Apply 3D audio processing
        // In a real implementation, this would apply various 3D audio effects
        // For now, we'll just do some basic processing
        
        // Get room parameters if source is in a room
        if (source.room_handle != 0 && is_valid_room_handle(source.room_handle)) {
            auto &room = rooms[source.room_handle];
            
            // Apply room effects
            // In a real implementation, this would apply reverb, reflections, etc.
        }
        
        // Clear source buffer after processing
        source.buffer.clear();
    }
}

bool SceAudio3dManager::is_valid_source_handle(SceAudio3dSourceHandle handle) const {
    return sources.find(handle) != sources.end();
}

bool SceAudio3dManager::is_valid_room_handle(SceAudio3dRoomHandle handle) const {
    return rooms.find(handle) != rooms.end();
}

bool SceAudio3dManager::is_valid_port_handle(SceAudio3dPortHandle handle) const {
    return ports.find(handle) != ports.end();
}

// Module function implementations
int sceAudio3dInit() {
    return g_audio3d_manager.initialize();
}

int sceAudio3dExit() {
    return g_audio3d_manager.terminate();
}

// Source functions
int sceAudio3dCreateSource(SceAudio3dSourceHandle *handle, const SceAudio3dSourceParam *params) {
    return g_audio3d_manager.create_source(handle, params);
}

int sceAudio3dReleaseSource(SceAudio3dSourceHandle handle) {
    return g_audio3d_manager.release_source(handle);
}

int sceAudio3dSetSourceParams(SceAudio3dSourceHandle handle, const SceAudio3dSourceParam *params) {
    return g_audio3d_manager.set_source_params(handle, params);
}

int sceAudio3dGetSourceParams(SceAudio3dSourceHandle handle, SceAudio3dSourceParam *params) {
    return g_audio3d_manager.get_source_params(handle, params);
}

int sceAudio3dSetSourceRoom(SceAudio3dSourceHandle handle, SceAudio3dRoomHandle room_handle) {
    return g_audio3d_manager.set_source_room(handle, room_handle);
}

int sceAudio3dOutputSource(SceAudio3dSourceHandle handle, const void *buffer, uint32_t samples) {
    return g_audio3d_manager.output_source(handle, buffer, samples);
}

// Listener functions
int sceAudio3dSetListenerParams(const SceAudio3dListenerParam *params) {
    return g_audio3d_manager.set_listener_params(params);
}

int sceAudio3dGetListenerParams(SceAudio3dListenerParam *params) {
    return g_audio3d_manager.get_listener_params(params);
}

// Room functions
int sceAudio3dCreateRoom(SceAudio3dRoomHandle *handle, const SceAudio3dRoomParam *params) {
    return g_audio3d_manager.create_room(handle, params);
}

int sceAudio3dReleaseRoom(SceAudio3dRoomHandle handle) {
    return g_audio3d_manager.release_room(handle);
}

int sceAudio3dSetRoomParams(SceAudio3dRoomHandle handle, const SceAudio3dRoomParam *params) {
    return g_audio3d_manager.set_room_params(handle, params);
}

int sceAudio3dGetRoomParams(SceAudio3dRoomHandle handle, SceAudio3dRoomParam *params) {
    return g_audio3d_manager.get_room_params(handle, params);
}

// Port functions
int sceAudio3dCreatePort(SceAudio3dPortHandle *handle, const SceAudio3dPortParam *params) {
    return g_audio3d_manager.create_port(handle, params);
}

int sceAudio3dReleasePort(SceAudio3dPortHandle handle) {
    return g_audio3d_manager.release_port(handle);
}

int sceAudio3dOutputPort(SceAudio3dPortHandle handle, void *buffer, uint32_t samples) {
    return g_audio3d_manager.output_port(handle, buffer, samples);
}

} // namespace audio
} // namespace modules
} // namespace firmware
