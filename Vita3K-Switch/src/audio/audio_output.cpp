#include "audio_output.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

AudioOutput::AudioOutput()
    : audio_device(0), initialized(false) {
}

AudioOutput::~AudioOutput() {
    finalize();
}

bool AudioOutput::initialize() {
    if (initialized) {
        return true;
    }
    
    printf("[AudioOutput] Initializing audio system\n");
    
    // Initialize SDL audio subsystem if not already initialized
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
            printf("[AudioOutput] Failed to initialize SDL audio: %s\n", SDL_GetError());
            return false;
        }
    }
    
    // Set up audio specification
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = AUDIO_SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = AUDIO_MAX_CHANNELS;
    want.samples = AUDIO_BUFFER_SIZE;
    want.callback = audioCallback;
    want.userdata = this;
    
    // Open audio device
    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_device == 0) {
        printf("[AudioOutput] Failed to open audio device: %s\n", SDL_GetError());
        return false;
    }
    
    // Start audio playback
    SDL_PauseAudioDevice(audio_device, 0);
    
    initialized = true;
    printf("[AudioOutput] Audio system initialized successfully\n");
    printf("[AudioOutput] Sample rate: %d Hz, Buffer size: %d samples, Channels: %d\n",
           have.freq, have.samples, have.channels);
    
    return true;
}

void AudioOutput::finalize() {
    if (!initialized) {
        return;
    }
    
    printf("[AudioOutput] Finalizing audio system\n");
    
    // Close all ports
    for (auto it = ports.begin(); it != ports.end(); ) {
        int port_id = it->id;
        it++; // Increment before closePort modifies the vector
        closePort(port_id);
    }
    
    // Close audio device
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
    
    initialized = false;
    printf("[AudioOutput] Audio system finalized\n");
}

int AudioOutput::openPort(int channel_count, int sample_rate, int buffer_size) {
    if (!initialized) {
        printf("[AudioOutput] Cannot open port: audio system not initialized\n");
        return -1;
    }
    
    // Validate parameters
    if (channel_count <= 0 || channel_count > AUDIO_MAX_CHANNELS) {
        printf("[AudioOutput] Invalid channel count: %d\n", channel_count);
        return -1;
    }
    
    if (sample_rate <= 0) {
        printf("[AudioOutput] Invalid sample rate: %d\n", sample_rate);
        return -1;
    }
    
    if (buffer_size <= 0) {
        printf("[AudioOutput] Invalid buffer size: %d\n", buffer_size);
        return -1;
    }
    
    // Check if we've reached the maximum number of ports
    if (ports.size() >= AUDIO_MAX_PORTS) {
        printf("[AudioOutput] Maximum number of ports reached\n");
        return -1;
    }
    
    // Create a new port
    SDL_LockAudioDevice(audio_device);
    
    ports.emplace_back();
    AudioPort& port = ports.back();
    port.id = generatePortId();
    port.active = true;
    port.channel_count = channel_count;
    port.sample_rate = sample_rate;
    port.buffer_size = buffer_size;
    port.buffer.resize(buffer_size * channel_count, 0);
    
    SDL_UnlockAudioDevice(audio_device);
    
    printf("[AudioOutput] Opened port %d: channels=%d, sample_rate=%d, buffer_size=%d\n",
           port.id, channel_count, sample_rate, buffer_size);
    
    return port.id;
}

bool AudioOutput::closePort(int port_id) {
    if (!initialized) {
        printf("[AudioOutput] Cannot close port: audio system not initialized\n");
        return false;
    }
    
    SDL_LockAudioDevice(audio_device);
    
    // Find the port
    for (auto it = ports.begin(); it != ports.end(); ++it) {
        if (it->id == port_id) {
            // Remove the port
            ports.erase(it);
            
            SDL_UnlockAudioDevice(audio_device);
            
            printf("[AudioOutput] Closed port %d\n", port_id);
            return true;
        }
    }
    
    SDL_UnlockAudioDevice(audio_device);
    
    printf("[AudioOutput] Port %d not found\n", port_id);
    return false;
}

bool AudioOutput::output(int port_id, const void* data, int size) {
    if (!initialized) {
        printf("[AudioOutput] Cannot output: audio system not initialized\n");
        return false;
    }
    
    if (!data || size <= 0) {
        printf("[AudioOutput] Invalid data or size\n");
        return false;
    }
    
    // Find the port
    AudioPort* port = findPort(port_id);
    if (!port) {
        printf("[AudioOutput] Port %d not found\n", port_id);
        return false;
    }
    
    // Lock the port's mutex
    std::lock_guard<std::mutex> lock(*port->mutex);
    
    // Copy the data to the port's buffer
    int samples = size / sizeof(int16_t);
    if (samples > port->buffer.size()) {
        samples = port->buffer.size();
    }
    
    memcpy(port->buffer.data(), data, samples * sizeof(int16_t));
    
    return true;
}

bool AudioOutput::setVolume(int port_id, float left_volume, float right_volume) {
    if (!initialized) {
        printf("[AudioOutput] Cannot set volume: audio system not initialized\n");
        return false;
    }
    
    // Clamp volume values
    left_volume = std::max(0.0f, std::min(1.0f, left_volume));
    right_volume = std::max(0.0f, std::min(1.0f, right_volume));
    
    // Find the port
    AudioPort* port = findPort(port_id);
    if (!port) {
        printf("[AudioOutput] Port %d not found\n", port_id);
        return false;
    }
    
    // TODO: Store and apply volume values
    
    printf("[AudioOutput] Set volume for port %d: left=%.2f, right=%.2f\n",
           port_id, left_volume, right_volume);
    
    return true;
}

bool AudioOutput::getPortInfo(int port_id, int* channel_count, int* sample_rate, int* buffer_size) {
    if (!initialized) {
        printf("[AudioOutput] Cannot get port info: audio system not initialized\n");
        return false;
    }
    
    // Find the port
    AudioPort* port = findPort(port_id);
    if (!port) {
        printf("[AudioOutput] Port %d not found\n", port_id);
        return false;
    }
    
    // Return the port info
    if (channel_count) *channel_count = port->channel_count;
    if (sample_rate) *sample_rate = port->sample_rate;
    if (buffer_size) *buffer_size = port->buffer_size;
    
    return true;
}

void AudioOutput::audioCallback(void* userdata, Uint8* stream, int len) {
    AudioOutput* audio = static_cast<AudioOutput*>(userdata);
    if (!audio) {
        return;
    }
    
    // Clear the output buffer
    memset(stream, 0, len);
    
    // Mix audio from all active ports
    int frames = len / (sizeof(int16_t) * AUDIO_MAX_CHANNELS);
    audio->mixAudio(reinterpret_cast<int16_t*>(stream), frames);
}

void AudioOutput::mixAudio(int16_t* output, int frames) {
    // Mix audio from all active ports
    for (auto& port : ports) {
        if (!port.active) {
            continue;
        }
        
        // Lock the port's mutex
        std::lock_guard<std::mutex> lock(*port.mutex);
        
        // Mix the port's audio data
        for (int i = 0; i < frames; i++) {
            for (int ch = 0; ch < port.channel_count; ch++) {
                int idx = i * AUDIO_MAX_CHANNELS + ch;
                int port_idx = i * port.channel_count + ch;
                
                if (port_idx < port.buffer.size()) {
                    // Simple mixing (add samples)
                    int32_t mixed = output[idx] + port.buffer[port_idx];
                    
                    // Clamp to prevent overflow
                    if (mixed > INT16_MAX) mixed = INT16_MAX;
                    if (mixed < INT16_MIN) mixed = INT16_MIN;
                    
                    output[idx] = static_cast<int16_t>(mixed);
                }
            }
        }
    }
}

AudioPort* AudioOutput::findPort(int port_id) {
    for (auto& port : ports) {
        if (port.id == port_id) {
            return &port;
        }
    }
    return nullptr;
}

int AudioOutput::generatePortId() {
    static int next_id = 1;
    return next_id++;
}
