#include "sceAudio.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace firmware {
namespace modules {
namespace audio {

// Audio format conversion functions for the SceAudio module

// Convert 16-bit PCM to 32-bit float
void convert_s16_to_f32(const int16_t *src, float *dst, int samples) {
    for (int i = 0; i < samples; i++) {
        dst[i] = src[i] / 32768.0f;
    }
}

// Convert 32-bit float to 16-bit PCM
void convert_f32_to_s16(const float *src, int16_t *dst, int samples) {
    for (int i = 0; i < samples; i++) {
        float sample = src[i] * 32768.0f;
        // Clamp to int16_t range
        sample = std::max(-32768.0f, std::min(32767.0f, sample));
        dst[i] = static_cast<int16_t>(sample);
    }
}

// Resample audio data
void resample_audio(const int16_t *src, int16_t *dst, int src_rate, int dst_rate, int channels, int samples) {
    // Simple linear resampling
    float ratio = static_cast<float>(src_rate) / dst_rate;
    int dst_samples = static_cast<int>(samples / ratio);
    
    for (int i = 0; i < dst_samples; i++) {
        float src_pos = i * ratio;
        int src_idx = static_cast<int>(src_pos);
        float frac = src_pos - src_idx;
        
        for (int c = 0; c < channels; c++) {
            int idx1 = src_idx * channels + c;
            int idx2 = (src_idx + 1) * channels + c;
            
            // Ensure we don't read past the end of the source buffer
            if (idx2 >= samples * channels) {
                idx2 = idx1;
            }
            
            // Linear interpolation
            float sample = src[idx1] * (1.0f - frac) + src[idx2] * frac;
            dst[i * channels + c] = static_cast<int16_t>(sample);
        }
    }
}

// Apply volume to audio data
void apply_volume(int16_t *buffer, int volume, int samples) {
    for (int i = 0; i < samples; i++) {
        buffer[i] = static_cast<int16_t>((static_cast<int32_t>(buffer[i]) * volume) / SCE_AUDIO_MAX_VOLUME);
    }
}

// Mix audio data
void mix_audio(const int16_t *src1, const int16_t *src2, int16_t *dst, int samples) {
    for (int i = 0; i < samples; i++) {
        // Mix samples and clamp to int16_t range
        int32_t mixed = static_cast<int32_t>(src1[i]) + static_cast<int32_t>(src2[i]);
        mixed = std::max(static_cast<int32_t>(-32768), std::min(static_cast<int32_t>(32767), mixed));
        dst[i] = static_cast<int16_t>(mixed);
    }
}

// Convert mono to stereo
void convert_mono_to_stereo(const int16_t *src, int16_t *dst, int samples) {
    for (int i = 0; i < samples; i++) {
        dst[i * 2] = src[i];     // Left channel
        dst[i * 2 + 1] = src[i]; // Right channel
    }
}

// Convert stereo to mono
void convert_stereo_to_mono(const int16_t *src, int16_t *dst, int samples) {
    for (int i = 0; i < samples; i++) {
        // Average left and right channels
        dst[i] = static_cast<int16_t>((static_cast<int32_t>(src[i * 2]) + static_cast<int32_t>(src[i * 2 + 1])) / 2);
    }
}

// Convert between different channel configurations
void convert_channels(const int16_t *src, int16_t *dst, int src_channels, int dst_channels, int samples) {
    if (src_channels == dst_channels) {
        // No conversion needed
        std::memcpy(dst, src, samples * src_channels * sizeof(int16_t));
    } else if (src_channels == 1 && dst_channels == 2) {
        // Mono to stereo
        convert_mono_to_stereo(src, dst, samples);
    } else if (src_channels == 2 && dst_channels == 1) {
        // Stereo to mono
        convert_stereo_to_mono(src, dst, samples);
    } else {
        // More complex channel conversions
        // For now, just copy the first dst_channels from each sample
        for (int i = 0; i < samples; i++) {
            for (int c = 0; c < dst_channels; c++) {
                if (c < src_channels) {
                    dst[i * dst_channels + c] = src[i * src_channels + c];
                } else {
                    dst[i * dst_channels + c] = 0;
                }
            }
        }
    }
}

// Apply 3D positioning to audio data
void apply_3d_positioning(int16_t *buffer, int channels, int samples, float x, float y, float z) {
    if (channels < 2) {
        // Need at least stereo for 3D positioning
        return;
    }
    
    // Calculate distance from listener (assumed to be at origin)
    float distance = std::sqrt(x*x + y*y + z*z);
    
    // Normalize direction vector
    if (distance > 0.0f) {
        x /= distance;
        y /= distance;
        z /= distance;
    } else {
        x = 0.0f;
        y = 0.0f;
        z = 1.0f;
    }
    
    // Calculate attenuation based on distance (1/r law)
    float attenuation = 1.0f;
    if (distance > 1.0f) {
        attenuation = 1.0f / distance;
    }
    
    // Calculate left/right panning based on x-coordinate
    // x = -1.0 (full left), x = 0.0 (center), x = 1.0 (full right)
    float pan = (x + 1.0f) / 2.0f; // Map [-1,1] to [0,1]
    
    // Calculate volume for left and right channels
    float left_volume = std::sqrt(1.0f - pan) * attenuation;
    float right_volume = std::sqrt(pan) * attenuation;
    
    // Apply panning and attenuation
    for (int i = 0; i < samples; i++) {
        int16_t sample = buffer[i * channels];
        
        // Apply volume to left and right channels
        buffer[i * channels] = static_cast<int16_t>(sample * left_volume);
        buffer[i * channels + 1] = static_cast<int16_t>(sample * right_volume);
        
        // For surround channels, apply different calculations based on position
        if (channels >= 4) {
            // Rear channels based on z-coordinate
            // z = -1.0 (behind), z = 0.0 (neutral), z = 1.0 (in front)
            float rear_factor = (1.0f - z) / 2.0f; // Map [-1,1] to [1,0]
            
            // Rear left and right
            buffer[i * channels + 2] = static_cast<int16_t>(sample * left_volume * rear_factor);
            buffer[i * channels + 3] = static_cast<int16_t>(sample * right_volume * rear_factor);
            
            // Center and LFE (if available)
            if (channels >= 6) {
                // Center channel (based on how centered the sound is)
                float center_factor = 1.0f - std::abs(x);
                buffer[i * channels + 4] = static_cast<int16_t>(sample * center_factor * attenuation);
                
                // LFE (low-frequency effects) - simplified approach
                // Apply low-pass filter (just attenuate for now)
                buffer[i * channels + 5] = static_cast<int16_t>(sample * 0.3f * attenuation);
            }
        }
    }
}

// Generate a sine wave for testing
void generate_sine_wave(int16_t *buffer, int frequency, int sample_rate, int channels, int samples) {
    float phase = 0.0f;
    float phase_increment = 2.0f * M_PI * frequency / sample_rate;
    
    for (int i = 0; i < samples; i++) {
        int16_t value = static_cast<int16_t>(32767.0f * std::sin(phase));
        
        for (int c = 0; c < channels; c++) {
            buffer[i * channels + c] = value;
        }
        
        phase += phase_increment;
        if (phase > 2.0f * M_PI) {
            phase -= 2.0f * M_PI;
        }
    }
}

// Apply a simple low-pass filter
void apply_low_pass_filter(int16_t *buffer, int channels, int samples, float cutoff, int sample_rate) {
    // Simple one-pole low-pass filter
    float alpha = 1.0f - std::exp(-2.0f * M_PI * cutoff / sample_rate);
    
    for (int c = 0; c < channels; c++) {
        float y = buffer[c]; // Initial state
        
        for (int i = 0; i < samples; i++) {
            int idx = i * channels + c;
            float x = buffer[idx];
            y = y + alpha * (x - y);
            buffer[idx] = static_cast<int16_t>(y);
        }
    }
}

// Apply a simple high-pass filter
void apply_high_pass_filter(int16_t *buffer, int channels, int samples, float cutoff, int sample_rate) {
    // Simple one-pole high-pass filter
    float alpha = 1.0f - std::exp(-2.0f * M_PI * cutoff / sample_rate);
    
    for (int c = 0; c < channels; c++) {
        float prev_x = buffer[c]; // Initial state
        float prev_y = 0.0f;
        
        for (int i = 0; i < samples; i++) {
            int idx = i * channels + c;
            float x = buffer[idx];
            float y = alpha * (prev_y + x - prev_x);
            buffer[idx] = static_cast<int16_t>(y);
            prev_x = x;
            prev_y = y;
        }
    }
}

// Apply a simple reverb effect
void apply_reverb(int16_t *buffer, int channels, int samples, float delay_time, float decay, int sample_rate) {
    int delay_samples = static_cast<int>(delay_time * sample_rate);
    if (delay_samples <= 0) {
        return;
    }
    
    // Create delay buffer
    std::vector<int16_t> delay_buffer(delay_samples * channels, 0);
    
    for (int i = 0; i < samples; i++) {
        for (int c = 0; c < channels; c++) {
            int idx = i * channels + c;
            int delay_idx = (i % delay_samples) * channels + c;
            
            // Read from delay buffer
            int16_t delayed = delay_buffer[delay_idx];
            
            // Apply reverb
            int32_t result = buffer[idx] + static_cast<int32_t>(delayed * decay);
            result = std::max(static_cast<int32_t>(-32768), std::min(static_cast<int32_t>(32767), result));
            buffer[idx] = static_cast<int16_t>(result);
            
            // Write to delay buffer
            delay_buffer[delay_idx] = buffer[idx];
        }
    }
}

// Apply a simple echo effect
void apply_echo(int16_t *buffer, int channels, int samples, float delay_time, float decay, int sample_rate) {
    int delay_samples = static_cast<int>(delay_time * sample_rate);
    if (delay_samples <= 0 || delay_samples >= samples) {
        return;
    }
    
    // Process from end to start to avoid overwriting data we need
    for (int i = samples - 1; i >= delay_samples; i--) {
        for (int c = 0; c < channels; c++) {
            int idx = i * channels + c;
            int delay_idx = (i - delay_samples) * channels + c;
            
            // Apply echo
            int32_t result = buffer[idx] + static_cast<int32_t>(buffer[delay_idx] * decay);
            result = std::max(static_cast<int32_t>(-32768), std::min(static_cast<int32_t>(32767), result));
            buffer[idx] = static_cast<int16_t>(result);
        }
    }
}

// Convert audio format
int convert_audio_format(const void *src, void *dst, int src_format, int dst_format, int src_rate, int dst_rate, int samples) {
    // Get channel counts
    int src_channels = get_channel_count(src_format);
    int dst_channels = get_channel_count(dst_format);
    
    // Temporary buffers for processing
    std::vector<int16_t> temp1(samples * std::max(src_channels, dst_channels));
    std::vector<int16_t> temp2(samples * std::max(src_channels, dst_channels));
    
    // Copy source data to temp buffer
    std::memcpy(temp1.data(), src, samples * src_channels * sizeof(int16_t));
    
    // Step 1: Convert sample rate if needed
    if (src_rate != dst_rate) {
        resample_audio(temp1.data(), temp2.data(), src_rate, dst_rate, src_channels, samples);
        // Update samples count after resampling
        samples = static_cast<int>(samples * static_cast<float>(dst_rate) / src_rate);
        // Swap buffers
        temp1.swap(temp2);
    }
    
    // Step 2: Convert channel configuration if needed
    if (src_channels != dst_channels) {
        convert_channels(temp1.data(), temp2.data(), src_channels, dst_channels, samples);
        // Swap buffers
        temp1.swap(temp2);
    }
    
    // Copy result to destination
    std::memcpy(dst, temp1.data(), samples * dst_channels * sizeof(int16_t));
    
    return samples;
}

// Get channel count from format
int get_channel_count(int format) {
    switch (format) {
        case SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO:
            return 1;
        case SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO:
            return 2;
        case SCE_AUDIO_OUT_PARAM_FORMAT_S16_QUAD:
            return 4;
        case SCE_AUDIO_OUT_PARAM_FORMAT_S16_5POINT1:
            return 6;
        case SCE_AUDIO_OUT_PARAM_FORMAT_S16_7POINT1:
            return 8;
        default:
            return 2; // Default to stereo
    }
}

} // namespace audio
} // namespace modules
} // namespace firmware
