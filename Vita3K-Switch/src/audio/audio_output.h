#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <SDL2/SDL.h>

// Audio constants
#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_BUFFER_SIZE 2048
#define AUDIO_MAX_CHANNELS 2
#define AUDIO_MAX_PORTS 8

// Audio port structure
struct AudioPort {
    int id;
    bool active;
    int channel_count;
    int sample_rate;
    int buffer_size;
    std::vector<int16_t> buffer;
    std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
    float left_volume; // Added for left channel volume
    float right_volume; // Added for right channel volume

    // Delete copy operations
    AudioPort(const AudioPort&) = delete;
    AudioPort& operator=(const AudioPort&) = delete;

    // Allow move operations
    AudioPort(AudioPort&&) noexcept = default;
    AudioPort& operator=(AudioPort&&) noexcept = default;

    // Default constructor
    AudioPort() = default;
};

class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();

    // Initialize the audio system
    bool initialize();
    
    // Finalize the audio system
    void finalize();
    
    // Open an audio port
    int openPort(int channel_count, int sample_rate, int buffer_size);
    
    // Close an audio port
    bool closePort(int port_id);
    
    // Output audio data to a port
    bool output(int port_id, const void* data, int size);
    
    // Set volume for a port
    bool setVolume(int port_id, float left_volume, float right_volume);
    
    // Get port info
    bool getPortInfo(int port_id, int* channel_count, int* sample_rate, int* buffer_size);

private:
    SDL_AudioDeviceID audio_device;
    std::vector<AudioPort> ports;
    bool initialized;
    
    // Audio callback function
    static void audioCallback(void* userdata, Uint8* stream, int len);
    
    // Mix audio data from all active ports
    void mixAudio(int16_t* output, int frames);
    
    // Find a port by ID
    AudioPort* findPort(int port_id);
    
    // Generate a unique port ID
    int generatePortId();
};
