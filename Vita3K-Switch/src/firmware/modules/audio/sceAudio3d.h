#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <cmath>

namespace firmware {
namespace modules {
namespace audio {

// 3D Audio room parameters
struct SceAudio3dRoomParam {
    float position[3];     // Position (x, y, z)
    float rotation[4];     // Rotation quaternion (x, y, z, w)
    float dimensions[3];   // Dimensions (width, height, depth)
    float reflection_ratio; // Reflection ratio (0.0 - 1.0)
    float reverb_time;     // Reverb time (seconds)
    float damping;         // Damping factor (0.0 - 1.0)
    float early_reflections; // Early reflections gain (0.0 - 1.0)
    float late_reverb;     // Late reverb gain (0.0 - 1.0)
};

// Forward declarations for effect functions
void apply_reverb(float *buffer, uint32_t samples, uint32_t channels, const firmware::modules::audio::SceAudio3dRoomParam &room_params);
void apply_muffled_effect(float *buffer, uint32_t samples, uint32_t channels, const firmware::modules::audio::SceAudio3dRoomParam &room_params);

// 3D Audio error codes
enum SceAudio3dErrorCode {
    SCE_AUDIO3D_ERROR_NOT_INITIALIZED    = 0x80850001,
    SCE_AUDIO3D_ERROR_ALREADY_INITIALIZED = 0x80850002,
    SCE_AUDIO3D_ERROR_INVALID_HANDLE     = 0x80850003,
    SCE_AUDIO3D_ERROR_INVALID_PARAMETER  = 0x80850004,
    SCE_AUDIO3D_ERROR_OUT_OF_MEMORY      = 0x80850005,
    SCE_AUDIO3D_ERROR_RESOURCE_BUSY      = 0x80850006,
    SCE_AUDIO3D_ERROR_NOT_SUPPORTED      = 0x80850007,
    SCE_AUDIO3D_ERROR_FATAL              = 0x808500FF
};

// 3D Audio constants
#define SCE_AUDIO3D_MAX_SOURCES          16
#define SCE_AUDIO3D_MAX_ROOMS            4
#define SCE_AUDIO3D_MAX_VIRTUAL_CHANNELS 8

// 3D Audio handle types
typedef uint32_t SceAudio3dSourceHandle;
typedef uint32_t SceAudio3dRoomHandle;
typedef uint32_t SceAudio3dPortHandle;

// 3D Audio source parameters
struct SceAudio3dSourceParam {
    float position[3];     // Position (x, y, z)
    float velocity[3];     // Velocity (vx, vy, vz)
    float direction[3];    // Direction (dx, dy, dz)
    float cone_inner_angle; // Inner angle of sound cone (degrees)
    float cone_outer_angle; // Outer angle of sound cone (degrees)
    float cone_outer_gain;  // Gain at outer cone
    float min_distance;    // Distance at which attenuation begins
    float max_distance;    // Distance at which attenuation ends
    float rolloff_factor;  // Rolloff factor for distance attenuation
    uint32_t channel_count; // Number of channels
    uint32_t sample_rate;   // Sample rate
};

// 3D Audio listener parameters
struct SceAudio3dListenerParam {
    float position[3];     // Position (x, y, z)
    float velocity[3];     // Velocity (vx, vy, vz)
    float forward[3];      // Forward direction (dx, dy, dz)
    float up[3];           // Up direction (ux, uy, uz)
};


// 3D Audio port parameters
struct SceAudio3dPortParam {
    uint32_t channel_count; // Number of channels
    uint32_t sample_rate;   // Sample rate
    uint32_t grain;         // Grain size
    uint32_t output_port;   // Output port ID
};

// 3D Audio source state
struct Audio3dSourceState {
    SceAudio3dSourceHandle handle;
    SceAudio3dSourceParam params;
    bool active;
    SceAudio3dRoomHandle room_handle;
    std::vector<float> buffer;
};

// 3D Audio room state
struct Audio3dRoomState {
    SceAudio3dRoomHandle handle;
    SceAudio3dRoomParam params;
    bool active;
    std::vector<SceAudio3dSourceHandle> sources;
};

// 3D Audio port state
struct Audio3dPortState {
    SceAudio3dPortHandle handle;
    SceAudio3dPortParam params;
    bool active;
    std::vector<int16_t> buffer;
};

// 3D Audio manager class
class SceAudio3dManager {
public:
    SceAudio3dManager();
    ~SceAudio3dManager();

    // Initialize the 3D audio manager
    int initialize();

    // Terminate the 3D audio manager
    int terminate();

    // Source functions
    int create_source(SceAudio3dSourceHandle *handle, const SceAudio3dSourceParam *params);
    int release_source(SceAudio3dSourceHandle handle);
    int set_source_params(SceAudio3dSourceHandle handle, const SceAudio3dSourceParam *params);
    int get_source_params(SceAudio3dSourceHandle handle, SceAudio3dSourceParam *params);
    int set_source_room(SceAudio3dSourceHandle handle, SceAudio3dRoomHandle room_handle);
    int output_source(SceAudio3dSourceHandle handle, const void *buffer, uint32_t samples);

    // Listener functions
    int set_listener_params(const SceAudio3dListenerParam *params);
    int get_listener_params(SceAudio3dListenerParam *params);

    // Room functions
    int create_room(SceAudio3dRoomHandle *handle, const SceAudio3dRoomParam *params);
    int release_room(SceAudio3dRoomHandle handle);
    int set_room_params(SceAudio3dRoomHandle handle, const SceAudio3dRoomParam *params);
    int get_room_params(SceAudio3dRoomHandle handle, SceAudio3dRoomParam *params);

    // Port functions
    int create_port(SceAudio3dPortHandle *handle, const SceAudio3dPortParam *params);
    int release_port(SceAudio3dPortHandle handle);
    int output_port(SceAudio3dPortHandle handle, void *buffer, uint32_t samples);

    // Process 3D audio
    void process_audio();

private:
    // 3D audio initialized flag
    bool initialized;

    // Listener parameters
    SceAudio3dListenerParam listener_params;

    // Sources
    std::map<SceAudio3dSourceHandle, Audio3dSourceState> sources;

    // Rooms
    std::map<SceAudio3dRoomHandle, Audio3dRoomState> rooms;

    // Ports
    std::map<SceAudio3dPortHandle, Audio3dPortState> ports;

    // Next handle values
    SceAudio3dSourceHandle next_source_handle;
    SceAudio3dRoomHandle next_room_handle;
    SceAudio3dPortHandle next_port_handle;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Helper functions
    bool is_valid_source_handle(SceAudio3dSourceHandle handle) const;
    bool is_valid_room_handle(SceAudio3dRoomHandle handle) const;
    bool is_valid_port_handle(SceAudio3dPortHandle handle) const;
    
    // 3D audio processing functions
    void apply_distance_attenuation(float *buffer, uint32_t samples, uint32_t channels, 
                                   const SceAudio3dSourceParam &source_params);
    void apply_directional_attenuation(float *buffer, uint32_t samples, uint32_t channels, 
                                      const SceAudio3dSourceParam &source_params);
    void apply_doppler_effect(float *buffer, uint32_t samples, uint32_t channels, 
                             const SceAudio3dSourceParam &source_params);
    void apply_room_effects(float *buffer, uint32_t samples, uint32_t channels, 
                           const SceAudio3dSourceParam &source_params, 
                           const SceAudio3dRoomParam &room_params);
    void spatialize_audio(float *buffer, uint32_t samples, uint32_t src_channels, uint32_t dst_channels, 
                         const SceAudio3dSourceParam &source_params);
};

// Module functions
int sceAudio3dInit();
int sceAudio3dExit();

// Source functions
int sceAudio3dCreateSource(SceAudio3dSourceHandle *handle, const SceAudio3dSourceParam *params);
int sceAudio3dReleaseSource(SceAudio3dSourceHandle handle);
int sceAudio3dSetSourceParams(SceAudio3dSourceHandle handle, const SceAudio3dSourceParam *params);
int sceAudio3dGetSourceParams(SceAudio3dSourceHandle handle, SceAudio3dSourceParam *params);
int sceAudio3dSetSourceRoom(SceAudio3dSourceHandle handle, SceAudio3dRoomHandle room_handle);
int sceAudio3dOutputSource(SceAudio3dSourceHandle handle, const void *buffer, uint32_t samples);

// Listener functions
int sceAudio3dSetListenerParams(const SceAudio3dListenerParam *params);
int sceAudio3dGetListenerParams(SceAudio3dListenerParam *params);

// Room functions
int sceAudio3dCreateRoom(SceAudio3dRoomHandle *handle, const SceAudio3dRoomParam *params);
int sceAudio3dReleaseRoom(SceAudio3dRoomHandle handle);
int sceAudio3dSetRoomParams(SceAudio3dRoomHandle handle, const SceAudio3dRoomParam *params);
int sceAudio3dGetRoomParams(SceAudio3dRoomHandle handle, SceAudio3dRoomParam *params);

// Port functions
int sceAudio3dCreatePort(SceAudio3dPortHandle *handle, const SceAudio3dPortParam *params);
int sceAudio3dReleasePort(SceAudio3dPortHandle handle);
int sceAudio3dOutputPort(SceAudio3dPortHandle handle, void *buffer, uint32_t samples);

} // namespace audio
} // namespace modules
} // namespace firmware
