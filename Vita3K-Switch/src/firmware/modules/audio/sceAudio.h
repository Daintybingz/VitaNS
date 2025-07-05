#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>

namespace firmware {
namespace modules {
namespace audio {

// Audio error codes
enum SceAudioErrorCode {
    SCE_AUDIO_ERROR_NOT_INITIALIZED    = 0x807F0001,
    SCE_AUDIO_ERROR_ALREADY_INITIALIZED = 0x807F0002,
    SCE_AUDIO_ERROR_BUSY               = 0x807F0003,
    SCE_AUDIO_ERROR_NOT_OPENED         = 0x807F0004,
    SCE_AUDIO_ERROR_INVALID_PORT       = 0x807F0005,
    SCE_AUDIO_ERROR_INVALID_SIZE       = 0x807F0006,
    SCE_AUDIO_ERROR_INVALID_FORMAT     = 0x807F0007,
    SCE_AUDIO_ERROR_INVALID_SAMPLE     = 0x807F0008,
    SCE_AUDIO_ERROR_INVALID_VOLUME     = 0x807F0009,
    SCE_AUDIO_ERROR_INVALID_POINTER    = 0x807F000A,
    SCE_AUDIO_ERROR_INVALID_PORT_TYPE  = 0x807F000B,
    SCE_AUDIO_ERROR_OUT_OF_MEMORY      = 0x807F000C,
    SCE_AUDIO_ERROR_NOT_SUPPORTED      = 0x807F000D,
    SCE_AUDIO_ERROR_FATAL              = 0x807F00FF
};

// Audio constants
#define SCE_AUDIO_MAX_VOLUME           0x8000
#define SCE_AUDIO_MIN_VOLUME           0x0000
#define SCE_AUDIO_VOLUME_0DB           0x8000

#define SCE_AUDIO_CHANNEL_OUT_MONO     0x1
#define SCE_AUDIO_CHANNEL_OUT_STEREO   0x2
#define SCE_AUDIO_CHANNEL_OUT_QUAD     0x4
#define SCE_AUDIO_CHANNEL_OUT_5POINT1  0x6
#define SCE_AUDIO_CHANNEL_OUT_7POINT1  0x8

#define SCE_AUDIO_SAMPLE_RATE_8K       8000
#define SCE_AUDIO_SAMPLE_RATE_11K      11025
#define SCE_AUDIO_SAMPLE_RATE_12K      12000
#define SCE_AUDIO_SAMPLE_RATE_16K      16000
#define SCE_AUDIO_SAMPLE_RATE_22K      22050
#define SCE_AUDIO_SAMPLE_RATE_24K      24000
#define SCE_AUDIO_SAMPLE_RATE_32K      32000
#define SCE_AUDIO_SAMPLE_RATE_44K      44100
#define SCE_AUDIO_SAMPLE_RATE_48K      48000

#define SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO    0
#define SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO  1
#define SCE_AUDIO_OUT_PARAM_FORMAT_S16_QUAD    2
#define SCE_AUDIO_OUT_PARAM_FORMAT_S16_5POINT1 3
#define SCE_AUDIO_OUT_PARAM_FORMAT_S16_7POINT1 4

#define SCE_AUDIO_OUT_PORT_TYPE_MAIN    0
#define SCE_AUDIO_OUT_PORT_TYPE_BGM     1
#define SCE_AUDIO_OUT_PORT_TYPE_VOICE   2
#define SCE_AUDIO_OUT_PORT_TYPE_VOICE_CHAT 3

#define SCE_AUDIO_MAX_PORTS             8

// Audio output port parameters
struct SceAudioOutPortParam {
    int format;
    int sample_rate;
    int channel_count;
    int grain;
    int reserved[4];
};

// Audio output port state
struct AudioOutPortState {
    int id;
    int channel_count;
    int sample_rate;
    int grain;
    int volume_left;
    int volume_right;
    int format;
    int type;
    bool is_active;
    std::queue<std::vector<int16_t>> buffer;
};

// Audio input port state
struct AudioInPortState {
    int id;
    int channel_count;
    int sample_rate;
    int grain;
    int volume;
    int format;
    bool is_active;
    std::vector<int16_t> buffer;
};

// Audio manager class
class SceAudioManager {
public:
    SceAudioManager();
    ~SceAudioManager();

    // Initialize the audio manager
    int initialize();

    // Terminate the audio manager
    int terminate();

    // Output port functions
    int open_output_port(int type, int grain, int sample_rate, int format);
    int release_output_port(int port);
    int output_audio(int port, const void *buffer);
    int output_audio_blocking(int port, const void *buffer);
    int set_output_volume(int port, int left_volume, int right_volume);
    int get_output_state(int port, SceAudioOutPortParam *param);
    
    // Input port functions
    int open_input_port(int type, int grain, int sample_rate, int format);
    int release_input_port(int port);
    int input_audio(int port, void *buffer);
    int input_audio_blocking(int port, void *buffer);
    int set_input_volume(int port, int volume);

    // Audio processing
    void process_audio();

private:
    // Audio initialized flag
    bool initialized;

    // Audio thread
    std::unique_ptr<std::thread> audio_thread;

    // Thread running flag
    std::atomic<bool> thread_running;

    // Output ports
    std::map<int, AudioOutPortState> output_ports;

    // Input ports
    std::map<int, AudioInPortState> input_ports;

    // Next port ID
    int next_port_id;

    // Mutex for thread safety
    mutable std::mutex mutex;

    // Audio thread function
    void audio_thread_func();

    // Helper functions
    bool is_valid_output_port(int port) const;
    bool is_valid_input_port(int port) const;
    bool is_valid_volume(int volume) const;
    bool is_valid_format(int format) const;
    bool is_valid_sample_rate(int sample_rate) const;
    bool is_valid_grain(int grain) const;
    int get_channel_count(int format) const;
};

// Module functions
int sceAudioInit();
int sceAudioExit();

// Output functions
int sceAudioOutOpenPort(int type, int grain, int sample_rate, int format);
int sceAudioOutReleasePort(int port);
int sceAudioOutOutput(int port, const void *buffer);
int sceAudioOutOutputBlocking(int port, const void *buffer);
int sceAudioOutSetVolume(int port, int left_volume, int right_volume);
int sceAudioOutSetConfig(int port, int grain, int sample_rate, int format);
int sceAudioOutGetConfig(int port, SceAudioOutPortParam *param);
int sceAudioOutGetRestSample(int port);
int sceAudioOutGetAdopt(int type);

// Input functions
int sceAudioInOpenPort(int type, int grain, int sample_rate, int format);
int sceAudioInReleasePort(int port);
int sceAudioInInput(int port, void *buffer);
int sceAudioInInputBlocking(int port, void *buffer);
int sceAudioInSetVolume(int port, int volume);
int sceAudioInGetAdopt(int type);

// Forward declaration for channel count helper
int get_channel_count(int format);

} // namespace audio
} // namespace modules
} // namespace firmware
