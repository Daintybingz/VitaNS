#include "sceAudio3d.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <complex>
#include <cstring>

namespace firmware {
namespace modules {
namespace audio {

// Vector operations for 3D audio processing
float dot_product(const float *a, const float *b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float vector_length(const float *v) {
    return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void normalize_vector(float *v) {
    float length = vector_length(v);
    if (length > 0.0f) {
        v[0] /= length;
        v[1] /= length;
        v[2] /= length;
    }
}

void cross_product(const float *a, const float *b, float *result) {
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
}

float distance_between(const float *a, const float *b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Apply distance attenuation to audio buffer
void SceAudio3dManager::apply_distance_attenuation(float *buffer, uint32_t samples, uint32_t channels, 
                                                 const SceAudio3dSourceParam &source_params) {
    // Calculate distance between source and listener
    float distance = distance_between(source_params.position, listener_params.position);
    
    // Calculate attenuation factor based on distance model
    float attenuation = 1.0f;
    
    if (distance <= source_params.min_distance) {
        // Within min distance, no attenuation
        attenuation = 1.0f;
    } else if (distance >= source_params.max_distance) {
        // Beyond max distance, use max attenuation
        if (source_params.max_distance > source_params.min_distance) {
            float range = source_params.max_distance - source_params.min_distance;
            attenuation = source_params.min_distance / (source_params.min_distance + source_params.rolloff_factor * range);
        } else {
            attenuation = 0.0f;
        }
    } else {
        // Between min and max distance, calculate attenuation
        float range = distance - source_params.min_distance;
        attenuation = source_params.min_distance / (source_params.min_distance + source_params.rolloff_factor * range);
    }
    
    // Apply attenuation to all samples
    for (uint32_t i = 0; i < samples * channels; i++) {
        buffer[i] *= attenuation;
    }
}

// Apply directional attenuation to audio buffer
void SceAudio3dManager::apply_directional_attenuation(float *buffer, uint32_t samples, uint32_t channels, 
                                                    const SceAudio3dSourceParam &source_params) {
    // Calculate direction from source to listener
    float direction[3];
    direction[0] = listener_params.position[0] - source_params.position[0];
    direction[1] = listener_params.position[1] - source_params.position[1];
    direction[2] = listener_params.position[2] - source_params.position[2];
    normalize_vector(direction);
    
    // Calculate angle between source direction and direction to listener
    float source_dir[3];
    memcpy(source_dir, source_params.direction, sizeof(float) * 3);
    normalize_vector(source_dir);
    
    float cos_angle = dot_product(source_dir, direction);
    float angle_rad = std::acos(std::max(-1.0f, std::min(1.0f, cos_angle)));
    float angle_deg = angle_rad * 180.0f / M_PI;
    
    // Calculate attenuation based on sound cone
    float attenuation = 1.0f;
    
    if (angle_deg <= source_params.cone_inner_angle) {
        // Within inner cone, no attenuation
        attenuation = 1.0f;
    } else if (angle_deg >= source_params.cone_outer_angle) {
        // Beyond outer cone, use outer gain
        attenuation = source_params.cone_outer_gain;
    } else {
        // Between inner and outer cone, interpolate
        float inner_rad = source_params.cone_inner_angle * M_PI / 180.0f;
        float outer_rad = source_params.cone_outer_angle * M_PI / 180.0f;
        float t = (angle_rad - inner_rad) / (outer_rad - inner_rad);
        attenuation = 1.0f + t * (source_params.cone_outer_gain - 1.0f);
    }
    
    // Apply attenuation to all samples
    for (uint32_t i = 0; i < samples * channels; i++) {
        buffer[i] *= attenuation;
    }
}

// Apply Doppler effect to audio buffer
void SceAudio3dManager::apply_doppler_effect(float *buffer, uint32_t samples, uint32_t channels, 
                                           const SceAudio3dSourceParam &source_params) {
    // Calculate direction from source to listener
    float direction[3];
    direction[0] = listener_params.position[0] - source_params.position[0];
    direction[1] = listener_params.position[1] - source_params.position[1];
    direction[2] = listener_params.position[2] - source_params.position[2];
    normalize_vector(direction);
    
    // Calculate relative velocity along the direction
    float source_vel_proj = dot_product(source_params.velocity, direction);
    float listener_vel_proj = dot_product(listener_params.velocity, direction);
    
    // Calculate Doppler factor
    const float SPEED_OF_SOUND = 343.3f; // m/s
    const float DOPPLER_FACTOR = 1.0f;
    
    float doppler_factor = (SPEED_OF_SOUND - DOPPLER_FACTOR * listener_vel_proj) / 
                          (SPEED_OF_SOUND - DOPPLER_FACTOR * source_vel_proj);
    
    // Clamp Doppler factor to reasonable range
    doppler_factor = std::max(0.5f, std::min(2.0f, doppler_factor));
    
    // If Doppler factor is close to 1.0, no need to apply effect
    if (std::abs(doppler_factor - 1.0f) < 0.01f) {
        return;
    }
    
    // Apply Doppler effect using resampling
    // This is a simplified implementation that would need to be replaced
    // with a proper resampling algorithm in a real implementation
    
    // For now, we'll just adjust the pitch by the Doppler factor
    // In a real implementation, this would be done using a high-quality resampler
    
    // This is just a placeholder for the actual implementation
    // In a real implementation, we would resample the audio buffer
    // based on the Doppler factor
}

// Apply room effects to audio buffer
void SceAudio3dManager::apply_room_effects(float *buffer, uint32_t samples, uint32_t channels, 
                                         const SceAudio3dSourceParam &source_params, 
                                         const SceAudio3dRoomParam &room_params) {
    // Check if source is inside the room
    bool inside_room = true;
    
    // Calculate source position relative to room
    float rel_pos[3];
    rel_pos[0] = source_params.position[0] - room_params.position[0];
    rel_pos[1] = source_params.position[1] - room_params.position[1];
    rel_pos[2] = source_params.position[2] - room_params.position[2];
    
    // Apply room rotation (simplified, should use quaternion in real implementation)
    // For now, we'll assume the room is axis-aligned
    
    // Check if source is inside room dimensions
    if (std::abs(rel_pos[0]) > room_params.dimensions[0] / 2.0f ||
        std::abs(rel_pos[1]) > room_params.dimensions[1] / 2.0f ||
        std::abs(rel_pos[2]) > room_params.dimensions[2] / 2.0f) {
        inside_room = false;
    }
    
    // Apply different effects based on whether source is inside or outside
    if (inside_room) {
        // Inside room, apply reverb based on room parameters
        apply_reverb(buffer, samples, channels, room_params);
    } else {
        // Outside room, apply muffled effect
        apply_muffled_effect(buffer, samples, channels, room_params);
    }
}

// Apply reverb effect based on room parameters
void apply_reverb(float *buffer, uint32_t samples, uint32_t channels, 
                 const SceAudio3dRoomParam &room_params) {
    // Calculate reverb parameters based on room dimensions and properties
    float room_volume = room_params.dimensions[0] * room_params.dimensions[1] * room_params.dimensions[2];
    float surface_area = 2.0f * (room_params.dimensions[0] * room_params.dimensions[1] +
                               room_params.dimensions[0] * room_params.dimensions[2] +
                               room_params.dimensions[1] * room_params.dimensions[2]);
    
    // Calculate average absorption coefficient
    float absorption = 1.0f - room_params.reflection_ratio;
    
    // Calculate reverberation time (RT60) using Sabine's formula
    float rt60 = 0.161f * room_volume / (surface_area * absorption);
    
    // Adjust RT60 based on room's reverb_time parameter
    rt60 *= room_params.reverb_time;
    
    // Calculate delay time for early reflections
    float early_delay = std::sqrt(room_volume) / 340.0f; // Speed of sound ~340 m/s
    
    // Create delay lines for early reflections and late reverb
    // In a real implementation, this would be more complex
    // For now, we'll just apply a simple feedback delay network
    
    // Create delay buffers
    const int NUM_DELAYS = 4;
    std::vector<std::vector<float>> delay_buffers(NUM_DELAYS);
    
    // Calculate delay times based on room dimensions
    std::vector<int> delay_samples(NUM_DELAYS);
    delay_samples[0] = static_cast<int>(early_delay * 44100.0f); // Assuming 44.1 kHz sample rate
    delay_samples[1] = static_cast<int>(early_delay * 1.5f * 44100.0f);
    delay_samples[2] = static_cast<int>(early_delay * 2.0f * 44100.0f);
    delay_samples[3] = static_cast<int>(early_delay * 2.5f * 44100.0f);
    
    // Initialize delay buffers
    for (int i = 0; i < NUM_DELAYS; i++) {
        delay_buffers[i].resize(delay_samples[i], 0.0f);
    }
    
    // Calculate feedback gains based on RT60
    std::vector<float> gains(NUM_DELAYS);
    for (int i = 0; i < NUM_DELAYS; i++) {
        gains[i] = std::pow(0.001f, delay_samples[i] / (rt60 * 44100.0f));
    }
    
    // Apply feedback delay network
    // This is a simplified implementation
    // In a real implementation, this would be more complex
    
    // For now, we'll just apply a simple gain to simulate reverb
    float reverb_gain = room_params.reflection_ratio * room_params.late_reverb;
    
    for (uint32_t i = 0; i < samples * channels; i++) {
        buffer[i] *= (1.0f + reverb_gain);
    }
}

// Apply muffled effect for sound coming from outside a room
void apply_muffled_effect(float *buffer, uint32_t samples, uint32_t channels, 
                         const SceAudio3dRoomParam &room_params) {
    // Apply low-pass filter to simulate sound passing through walls
    // The cutoff frequency depends on the room's reflection ratio
    // Higher reflection ratio means more high frequencies are blocked
    
    float cutoff = 20000.0f * (1.0f - room_params.reflection_ratio);
    cutoff = std::max(200.0f, std::min(20000.0f, cutoff));
    
    // Apply simple one-pole low-pass filter
    float alpha = 1.0f - std::exp(-2.0f * M_PI * cutoff / 44100.0f); // Assuming 44.1 kHz sample rate
    
    float y_prev = buffer[0];
    
    for (uint32_t i = 0; i < samples * channels; i++) {
        float x = buffer[i];
        float y = y_prev + alpha * (x - y_prev);
        buffer[i] = y;
        y_prev = y;
    }
    
    // Also reduce volume based on room's reflection ratio
    float attenuation = 1.0f - room_params.reflection_ratio * 0.8f;
    
    for (uint32_t i = 0; i < samples * channels; i++) {
        buffer[i] *= attenuation;
    }
}

// Spatialize audio based on source position relative to listener
void SceAudio3dManager::spatialize_audio(float *buffer, uint32_t samples, uint32_t src_channels, uint32_t dst_channels, 
                                       const SceAudio3dSourceParam &source_params) {
    // Calculate direction from listener to source
    float direction[3];
    direction[0] = source_params.position[0] - listener_params.position[0];
    direction[1] = source_params.position[1] - listener_params.position[1];
    direction[2] = source_params.position[2] - listener_params.position[2];
    
    float distance = vector_length(direction);
    
    if (distance > 0.0f) {
        direction[0] /= distance;
        direction[1] /= distance;
        direction[2] /= distance;
    } else {
        direction[0] = 0.0f;
        direction[1] = 0.0f;
        direction[2] = 1.0f;
    }
    
    // Calculate listener's coordinate system
    float forward[3], up[3], right[3];
    memcpy(forward, listener_params.forward, sizeof(float) * 3);
    memcpy(up, listener_params.up, sizeof(float) * 3);
    normalize_vector(forward);
    normalize_vector(up);
    
    // Calculate right vector using cross product
    cross_product(forward, up, right);
    normalize_vector(right);
    
    // Recalculate up vector to ensure orthogonality
    cross_product(right, forward, up);
    normalize_vector(up);
    
    // Project direction vector onto listener's coordinate system
    float proj_right = dot_product(direction, right);
    float proj_up = dot_product(direction, up);
    float proj_forward = dot_product(direction, forward);
    
    // Calculate azimuth and elevation
    float azimuth = std::atan2(proj_right, proj_forward);
    float elevation = std::atan2(proj_up, std::sqrt(proj_forward * proj_forward + proj_right * proj_right));
    
    // Convert to degrees
    azimuth = azimuth * 180.0f / M_PI;
    elevation = elevation * 180.0f / M_PI;
    
    // Calculate gains for each channel based on HRTF
    // This is a simplified implementation
    // In a real implementation, this would use a proper HRTF
    
    std::vector<float> gains(dst_channels, 0.0f);
    
    if (dst_channels == 1) {
        // Mono output, just use distance attenuation
        gains[0] = 1.0f;
    } else if (dst_channels == 2) {
        // Stereo output, use simple panning
        // Map azimuth from [-180, 180] to [0, 1]
        float pan = (azimuth + 180.0f) / 360.0f;
        
        // Apply constant power panning
        gains[0] = std::cos(pan * M_PI / 2.0f); // Left
        gains[1] = std::sin(pan * M_PI / 2.0f); // Right
    } else if (dst_channels >= 4) {
        // Surround output, use more complex spatialization
        // This is a simplified implementation
        // In a real implementation, this would be more complex
        
        // Front left/right
        float front_pan = (azimuth + 180.0f) / 360.0f;
        gains[0] = std::max(0.0f, static_cast<float>(std::cos(front_pan * M_PI))); // Front left
        gains[1] = std::max(0.0f, static_cast<float>(std::sin(front_pan * M_PI))); // Front right
        
        // Rear left/right
        float rear_pan = (azimuth + 180.0f) / 360.0f;
        gains[2] = std::max(0.0f, static_cast<float>(std::cos(rear_pan * M_PI))); // Rear left
        gains[3] = std::max(0.0f, static_cast<float>(std::sin(rear_pan * M_PI))); // Rear right
        
        // Adjust front/rear balance based on elevation
        float front_weight = (elevation + 90.0f) / 180.0f;
        float rear_weight = 1.0f - front_weight;
        
        gains[0] *= front_weight;
        gains[1] *= front_weight;
        gains[2] *= rear_weight;
        gains[3] *= rear_weight;
        
        if (dst_channels >= 6) {
            // Center and LFE
            gains[4] = std::max(0.0f, 1.0f - std::abs(azimuth) / 90.0f); // Center
            gains[5] = 1.0f; // LFE (will be filtered later)
        }
    }
    
    // Apply gains to buffer
    // This assumes the input buffer has the same number of channels as the source
    // and the output buffer has dst_channels
    
    // Create output buffer
    std::vector<float> output(samples * dst_channels, 0.0f);
    
    // Mix input channels to output channels using gains
    for (uint32_t i = 0; i < samples; i++) {
        for (uint32_t src_ch = 0; src_ch < src_channels; src_ch++) {
            float sample = buffer[i * src_channels + src_ch];
            
            for (uint32_t dst_ch = 0; dst_ch < dst_channels; dst_ch++) {
                output[i * dst_channels + dst_ch] += sample * gains[dst_ch];
            }
        }
    }
    
    // Copy output buffer back to input buffer
    // This assumes the input buffer is large enough to hold the output
    memcpy(buffer, output.data(), samples * dst_channels * sizeof(float));
}

// Apply HRTF (Head-Related Transfer Function) to simulate 3D audio
void apply_hrtf(float *buffer, uint32_t samples, uint32_t channels, 
               float azimuth, float elevation) {
    // This is a placeholder for a real HRTF implementation
    // In a real implementation, this would use a proper HRTF database
    // and apply convolution with the appropriate impulse responses
    
    // For now, we'll just apply a simple filter based on azimuth and elevation
    
    // Calculate interaural time difference (ITD)
    const float HEAD_RADIUS = 0.0875f; // meters
    const float SPEED_OF_SOUND = 343.3f; // m/s
    
    float azimuth_rad = azimuth * M_PI / 180.0f;
    float itd = HEAD_RADIUS * (azimuth_rad + std::sin(azimuth_rad)) / SPEED_OF_SOUND;
    
    // Calculate interaural level difference (ILD)
    float ild_db = 5.0f * std::sin(azimuth_rad);
    float ild = std::pow(10.0f, ild_db / 20.0f);
    
    // Apply ITD and ILD
    if (channels >= 2) {
        // Calculate delay in samples
        float sample_rate = 44100.0f; // Assuming 44.1 kHz sample rate
        int delay_samples = static_cast<int>(std::abs(itd) * sample_rate);
        
        // Apply delay and level difference
        if (azimuth < 0.0f) {
            // Sound comes from left, delay right channel
            for (uint32_t i = samples - 1; i >= delay_samples; i--) {
                buffer[i * channels + 1] = buffer[(i - delay_samples) * channels + 1] / ild;
            }
            for (uint32_t i = 0; i < delay_samples; i++) {
                buffer[i * channels + 1] = 0.0f;
            }
        } else {
            // Sound comes from right, delay left channel
            for (uint32_t i = samples - 1; i >= delay_samples; i--) {
                buffer[i * channels] = buffer[(i - delay_samples) * channels] / ild;
            }
            for (uint32_t i = 0; i < delay_samples; i++) {
                buffer[i * channels] = 0.0f;
            }
        }
    }
}

} // namespace audio
} // namespace modules
} // namespace firmware
