#pragma once
#include "../module.h"
#include "../../audio/audio_output.h"
#include <cstdint>
#include <vector>
#include <mutex>
#include <memory>

// Audio port status
#define SCE_AUDIO_OUT_PORT_TYPE_MAIN    0
#define SCE_AUDIO_OUT_PORT_TYPE_BGM     1
#define SCE_AUDIO_OUT_PORT_TYPE_VOICE   2

// Audio sampling rates
#define SCE_AUDIO_SAMPLE_RATE_44100     44100
#define SCE_AUDIO_SAMPLE_RATE_48000     48000

// Audio channel configuration
#define SCE_AUDIO_OUT_MODE_MONO         0
#define SCE_AUDIO_OUT_MODE_STEREO       1

// Audio port states
#define SCE_AUDIO_OUT_STATE_CLOSED      0
#define SCE_AUDIO_OUT_STATE_OPENED      1

// Audio port structure
struct SceAudioPort {
    int id;
    int channel_count;
    int sample_rate;
    int state;
    int type;
    int len;
    std::vector<int16_t> buffer;
};

class SceAudio : public Module {
public:
    SceAudio();
    virtual ~SceAudio();
    void registerFunctions() override;
    
    // Initialize the audio module
    bool initialize();
    
    // Finalize the audio module
    void finalize();
    
    // Get the audio output system
    static AudioOutput* getAudioOutput();

    // System call implementations
    static int sceAudioOutOpenPort(int type, int len, int sample_rate, int mode);
    static int sceAudioOutOutput(int port, const void* buf);
    static int sceAudioOutReleasePort(int port);
    static int sceAudioOutSetVolume(int port, int ch, int *vol);
    static int sceAudioOutSetConfig(int port, int len, int freq, int mode);
    static int sceAudioOutGetConfig(int port, int *len, int *freq, int *mode);

private:
    // Port mapping (PS Vita port ID -> Switch audio port ID)
    struct PortMapping {
        int vita_port_id;
        int switch_port_id;
        int type;
        int len;
        int sample_rate;
        int mode;
    };
    
    static std::vector<PortMapping> portMappings;
    static std::mutex audioMutex;
    static std::unique_ptr<AudioOutput> audioOutput;
    static int nextPortId;

    // Helper functions
    static PortMapping* getPortMapping(int vita_port_id);
    static void initializeAudio();
    static void finalizeAudio();
};
