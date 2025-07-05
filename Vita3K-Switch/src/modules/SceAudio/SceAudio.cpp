#include "SceAudio.h"
#include "../module_registry.h"
#include <cstdio>
#include <cstring>

// Initialize static members
std::vector<SceAudio::PortMapping> SceAudio::portMappings;
std::mutex SceAudio::audioMutex;
std::unique_ptr<AudioOutput> SceAudio::audioOutput = nullptr;
int SceAudio::nextPortId = 0;

SceAudio::SceAudio() : Module("SceAudio") {
    // Create the audio output system if it doesn't exist
    if (!audioOutput) {
        audioOutput = std::make_unique<AudioOutput>();
    }
}

SceAudio::~SceAudio() {
    finalize();
}

void SceAudio::registerFunctions() {
    registerFunction("sceAudioOutOpenPort", 0x5BC341E4,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: type, len, sample_rate, mode
            if (args.size() < 4) return -1;
            return SceAudio::sceAudioOutOpenPort(args[0], args[1], args[2], args[3]);
        },
        4
    );
    registerFunction("sceAudioOutOutput", 0x02DB3F5F,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port, buf (as pointer)
            if (args.size() < 2) return -1;
            return SceAudio::sceAudioOutOutput(args[0], reinterpret_cast<const void*>(args[1]));
        },
        2
    );
    registerFunction("sceAudioOutReleasePort", 0x69E2E6B5,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port
            if (args.size() < 1) return -1;
            return SceAudio::sceAudioOutReleasePort(args[0]);
        },
        1
    );
    registerFunction("sceAudioOutSetVolume", 0x64167F11,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port, ch, vol (as pointer)
            if (args.size() < 3) return -1;
            return SceAudio::sceAudioOutSetVolume(args[0], args[1], reinterpret_cast<int*>(args[2]));
        },
        3
    );
    registerFunction("sceAudioOutSetConfig", 0xB7F1D8CA,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port, len, freq, mode
            if (args.size() < 4) return -1;
            return SceAudio::sceAudioOutSetConfig(args[0], args[1], args[2], args[3]);
        },
        4
    );
    registerFunction("sceAudioOutGetConfig", 0x80260C26,
        [](Emulator&, uint32_t /*unused*/, const std::vector<uint32_t>& args) -> int {
            // args: port, len (ptr), freq (ptr), mode (ptr)
            if (args.size() < 4) return -1;
            return SceAudio::sceAudioOutGetConfig(args[0], reinterpret_cast<int*>(args[1]), reinterpret_cast<int*>(args[2]), reinterpret_cast<int*>(args[3]));
        },
        4
    );
}

bool SceAudio::initialize() {
    printf("[SceAudio] Initializing audio module\n");
    
    if (!audioOutput) {
        printf("[SceAudio] Error: Audio output is null\n");
        return false;
    }
    
    // Initialize the audio output system
    if (!audioOutput->initialize()) {
        printf("[SceAudio] Failed to initialize audio output\n");
        return false;
    }
    
    // Initialize audio system
    initializeAudio();
    
    printf("[SceAudio] Audio module initialized successfully\n");
    return true;
}

void SceAudio::finalize() {
    printf("[SceAudio] Finalizing audio module\n");
    
    // Finalize audio system
    finalizeAudio();
    
    // Finalize the audio output system
    if (audioOutput) {
        audioOutput->finalize();
    }
}

AudioOutput* SceAudio::getAudioOutput() {
    return audioOutput.get();
}

void SceAudio::initializeAudio() {
    printf("[SceAudio] Initializing audio system\n");
    
    // Clear any existing port mappings
    std::lock_guard<std::mutex> lock(audioMutex);
    portMappings.clear();
    nextPortId = 0;
}

void SceAudio::finalizeAudio() {
    printf("[SceAudio] Finalizing audio system\n");
    
    // Close all ports
    std::lock_guard<std::mutex> lock(audioMutex);
    for (auto& mapping : portMappings) {
        if (audioOutput) {
            printf("[SceAudio] Auto-closing port %d (Switch port %d)\n", mapping.vita_port_id, mapping.switch_port_id);
            audioOutput->closePort(mapping.switch_port_id);
        }
    }
    
    portMappings.clear();
}

SceAudio::PortMapping* SceAudio::getPortMapping(int vita_port_id) {
    for (auto& mapping : portMappings) {
        if (mapping.vita_port_id == vita_port_id) {
            return &mapping;
        }
    }
    return nullptr;
}

int SceAudio::sceAudioOutOpenPort(int type, int len, int sample_rate, int mode) {
    printf("[SceAudio] sceAudioOutOpenPort called: type=%d, len=%d, sample_rate=%d, mode=%d\n", 
           type, len, sample_rate, mode);
    
    // Validate parameters
    if (type < 0 || type > 2) {
        printf("[SceAudio] Error: Invalid port type %d\n", type);
        return -1;
    }
    
    if (len <= 0) {
        printf("[SceAudio] Error: Invalid buffer length %d\n", len);
        return -1;
    }
    
    if (sample_rate != SCE_AUDIO_SAMPLE_RATE_44100 && sample_rate != SCE_AUDIO_SAMPLE_RATE_48000) {
        printf("[SceAudio] Error: Invalid sample rate %d\n", sample_rate);
        return -1;
    }
    
    if (mode != SCE_AUDIO_OUT_MODE_MONO && mode != SCE_AUDIO_OUT_MODE_STEREO) {
        printf("[SceAudio] Error: Invalid mode %d\n", mode);
        return -1;
    }
    
    if (!audioOutput) {
        printf("[SceAudio] Error: Audio output is null\n");
        return -1;
    }
    
    // Create new port mapping
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Calculate channel count
    int channel_count = (mode == SCE_AUDIO_OUT_MODE_MONO) ? 1 : 2;
    
    // Open a port in the audio output system
    int switch_port_id = audioOutput->openPort(channel_count, sample_rate, len);
    if (switch_port_id < 0) {
        printf("[SceAudio] Error: Failed to open audio port\n");
        return -1;
    }
    
    // Create a port mapping
    PortMapping mapping;
    mapping.vita_port_id = nextPortId++;
    mapping.switch_port_id = switch_port_id;
    mapping.type = type;
    mapping.len = len;
    mapping.sample_rate = sample_rate;
    mapping.mode = mode;
    
    portMappings.push_back(mapping);
    
    printf("[SceAudio] Opened port %d (Switch port %d)\n", mapping.vita_port_id, switch_port_id);
    
    return mapping.vita_port_id;
}

int SceAudio::sceAudioOutOutput(int port, const void* buf) {
    printf("[SceAudio] sceAudioOutOutput called: port=%d, buf=%p\n", port, buf);
    
    if (!buf) {
        printf("[SceAudio] Error: buf is NULL\n");
        return -1;
    }
    
    if (!audioOutput) {
        printf("[SceAudio] Error: Audio output is null\n");
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Find the port mapping
    PortMapping* mapping = getPortMapping(port);
    if (!mapping) {
        printf("[SceAudio] Error: Invalid port %d\n", port);
        return -1;
    }
    
    // Calculate buffer size
    int channel_count = (mapping->mode == SCE_AUDIO_OUT_MODE_MONO) ? 1 : 2;
    int buffer_size = mapping->len * channel_count * sizeof(int16_t);
    
    // Output audio data to the audio output system
    if (!audioOutput->output(mapping->switch_port_id, buf, buffer_size)) {
        printf("[SceAudio] Error: Failed to output audio data\n");
        return -1;
    }
    
    return 0;
}

int SceAudio::sceAudioOutReleasePort(int port) {
    printf("[SceAudio] sceAudioOutReleasePort called: port=%d\n", port);
    
    if (!audioOutput) {
        printf("[SceAudio] Error: Audio output is null\n");
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Find the port mapping
    for (auto it = portMappings.begin(); it != portMappings.end(); ++it) {
        if (it->vita_port_id == port) {
            // Close the port in the audio output system
            if (!audioOutput->closePort(it->switch_port_id)) {
                printf("[SceAudio] Warning: Failed to close audio port %d\n", it->switch_port_id);
            }
            
            // Remove the port mapping
            portMappings.erase(it);
            
            printf("[SceAudio] Released port %d\n", port);
            return 0;
        }
    }
    
    printf("[SceAudio] Error: Invalid port %d\n", port);
    return -1;
}

int SceAudio::sceAudioOutSetVolume(int port, int ch, int *vol) {
    printf("[SceAudio] sceAudioOutSetVolume called: port=%d, ch=%d\n", port, ch);
    
    if (!vol) {
        printf("[SceAudio] Error: vol is NULL\n");
        return -1;
    }
    
    if (!audioOutput) {
        printf("[SceAudio] Error: Audio output is null\n");
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Find the port mapping
    PortMapping* mapping = getPortMapping(port);
    if (!mapping) {
        printf("[SceAudio] Error: Invalid port %d\n", port);
        return -1;
    }
    
    // Convert volume from PS Vita format (0-32767) to float (0.0-1.0)
    float left_volume = vol[0] / 32767.0f;
    float right_volume = vol[1] / 32767.0f;
    
    // Set volume in the audio output system
    if (!audioOutput->setVolume(mapping->switch_port_id, left_volume, right_volume)) {
        printf("[SceAudio] Error: Failed to set volume\n");
        return -1;
    }
    
    printf("[SceAudio] Set volume for port %d: left=%.2f, right=%.2f\n", 
           port, left_volume, right_volume);
    
    return 0;
}

int SceAudio::sceAudioOutSetConfig(int port, int len, int freq, int mode) {
    printf("[SceAudio] sceAudioOutSetConfig called: port=%d, len=%d, freq=%d, mode=%d\n", 
           port, len, freq, mode);
    
    if (!audioOutput) {
        printf("[SceAudio] Error: Audio output is null\n");
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Find the port mapping
    PortMapping* mapping = getPortMapping(port);
    if (!mapping) {
        printf("[SceAudio] Error: Invalid port %d\n", port);
        return -1;
    }
    
    // Validate parameters
    if (len <= 0) {
        printf("[SceAudio] Error: Invalid buffer length %d\n", len);
        return -1;
    }
    
    if (freq != SCE_AUDIO_SAMPLE_RATE_44100 && freq != SCE_AUDIO_SAMPLE_RATE_48000) {
        printf("[SceAudio] Error: Invalid sample rate %d\n", freq);
        return -1;
    }
    
    if (mode != SCE_AUDIO_OUT_MODE_MONO && mode != SCE_AUDIO_OUT_MODE_STEREO) {
        printf("[SceAudio] Error: Invalid mode %d\n", mode);
        return -1;
    }
    
    // Close the old port
    if (!audioOutput->closePort(mapping->switch_port_id)) {
        printf("[SceAudio] Warning: Failed to close audio port %d\n", mapping->switch_port_id);
    }
    
    // Calculate channel count
    int channel_count = (mode == SCE_AUDIO_OUT_MODE_MONO) ? 1 : 2;
    
    // Open a new port with the new configuration
    int switch_port_id = audioOutput->openPort(channel_count, freq, len);
    if (switch_port_id < 0) {
        printf("[SceAudio] Error: Failed to open audio port\n");
        return -1;
    }
    
    // Update the port mapping
    mapping->switch_port_id = switch_port_id;
    mapping->len = len;
    mapping->sample_rate = freq;
    mapping->mode = mode;
    
    return 0;
}

int SceAudio::sceAudioOutGetConfig(int port, int *len, int *freq, int *mode) {
    printf("[SceAudio] sceAudioOutGetConfig called: port=%d\n", port);
    
    if (!len || !freq || !mode) {
        printf("[SceAudio] Error: Invalid pointers\n");
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(audioMutex);
    
    // Find the port mapping
    PortMapping* mapping = getPortMapping(port);
    if (!mapping) {
        printf("[SceAudio] Error: Invalid port %d\n", port);
        return -1;
    }
    
    // Return the port configuration
    *len = mapping->len;
    *freq = mapping->sample_rate;
    *mode = mapping->mode;
    
    return 0;
}
