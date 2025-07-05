#include "sceCamera.h"
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace firmware {
namespace modules {
namespace camera {

// Helper function to convert framerate enum to actual FPS
int get_fps_from_framerate(int framerate) {
    switch (framerate) {
        case SCE_CAMERA_FRAMERATE_3_FPS:
            return 3;
        case SCE_CAMERA_FRAMERATE_5_FPS:
            return 5;
        case SCE_CAMERA_FRAMERATE_7_5_FPS:
            return 7;
        case SCE_CAMERA_FRAMERATE_10_FPS:
            return 10;
        case SCE_CAMERA_FRAMERATE_15_FPS:
            return 15;
        case SCE_CAMERA_FRAMERATE_20_FPS:
            return 20;
        case SCE_CAMERA_FRAMERATE_30_FPS:
            return 30;
        case SCE_CAMERA_FRAMERATE_60_FPS:
            return 60;
        default:
            return 30;
    }
}

// Helper function to get resolution dimensions
void get_resolution_dimensions(int resolution, int* width, int* height) {
    switch (resolution) {
        case SCE_CAMERA_RESOLUTION_640_480:
            *width = 640;
            *height = 480;
            break;
        case SCE_CAMERA_RESOLUTION_320_240:
            *width = 320;
            *height = 240;
            break;
        case SCE_CAMERA_RESOLUTION_160_120:
            *width = 160;
            *height = 120;
            break;
        case SCE_CAMERA_RESOLUTION_352_288:
            *width = 352;
            *height = 288;
            break;
        case SCE_CAMERA_RESOLUTION_176_144:
            *width = 176;
            *height = 144;
            break;
        case SCE_CAMERA_RESOLUTION_480_272:
            *width = 480;
            *height = 272;
            break;
        case SCE_CAMERA_RESOLUTION_640_360:
            *width = 640;
            *height = 360;
            break;
        default:
            *width = 640;
            *height = 480;
            break;
    }
}

// Helper function to calculate frame size based on format and dimensions
int calculate_frame_size(int format, int width, int height) {
    switch (format) {
        case SCE_CAMERA_FORMAT_YUV420:
            // YUV420: Y plane (width * height) + U/V planes (width/2 * height/2 * 2)
            return width * height + (width / 2) * (height / 2) * 2;
        case SCE_CAMERA_FORMAT_YUV422:
        case SCE_CAMERA_FORMAT_YUVU422:
            // YUV422: 2 bytes per pixel
            return width * height * 2;
        case SCE_CAMERA_FORMAT_ARGB:
        case SCE_CAMERA_FORMAT_ABGR:
            // ARGB/ABGR: 4 bytes per pixel
            return width * height * 4;
        case SCE_CAMERA_FORMAT_RAW8:
            // RAW8: 1 byte per pixel
            return width * height;
        default:
            return width * height * 2;
    }
}

// Helper function to apply camera effects to YUV420 frame
void apply_effect_yuv420(uint8_t* frame_data, int width, int height, int effect) {
    int y_plane_size = width * height;
    uint8_t* y_plane = frame_data;
    uint8_t* u_plane = y_plane + y_plane_size;
    uint8_t* v_plane = u_plane + (y_plane_size / 4);
    
    switch (effect) {
        case SCE_CAMERA_EFFECT_NEGATIVE:
            // Invert Y plane (luminance)
            for (int i = 0; i < y_plane_size; i++) {
                y_plane[i] = 255 - y_plane[i];
            }
            break;
            
        case SCE_CAMERA_EFFECT_BLACKWHITE:
            // Set U and V planes to 128 (no color)
            std::fill(u_plane, u_plane + (y_plane_size / 4), 128);
            std::fill(v_plane, v_plane + (y_plane_size / 4), 128);
            
            // Threshold Y plane for black and white effect
            for (int i = 0; i < y_plane_size; i++) {
                y_plane[i] = (y_plane[i] > 128) ? 255 : 0;
            }
            break;
            
        case SCE_CAMERA_EFFECT_SEPIA:
            // Set U and V planes for sepia tone
            std::fill(u_plane, u_plane + (y_plane_size / 4), 90);  // U = 90 for sepia
            std::fill(v_plane, v_plane + (y_plane_size / 4), 120); // V = 120 for sepia
            break;
            
        case SCE_CAMERA_EFFECT_BLUE:
            // Set U and V planes for blue tint
            std::fill(u_plane, u_plane + (y_plane_size / 4), 200); // U = 200 for blue
            std::fill(v_plane, v_plane + (y_plane_size / 4), 128); // V = 128 (neutral)
            break;
            
        case SCE_CAMERA_EFFECT_RED:
            // Set U and V planes for red tint
            std::fill(u_plane, u_plane + (y_plane_size / 4), 128); // U = 128 (neutral)
            std::fill(v_plane, v_plane + (y_plane_size / 4), 200); // V = 200 for red
            break;
            
        case SCE_CAMERA_EFFECT_GREEN:
            // Set U and V planes for green tint
            std::fill(u_plane, u_plane + (y_plane_size / 4), 50);  // U = 50 for green
            std::fill(v_plane, v_plane + (y_plane_size / 4), 50);  // V = 50 for green
            break;
    }
}

// Helper function to adjust brightness of YUV420 frame
void adjust_brightness_yuv420(uint8_t* frame_data, int width, int height, int brightness) {
    // Normalize brightness from 0-255 to -128-127
    int brightness_offset = brightness - 128;
    
    // Only adjust Y plane (luminance)
    int y_plane_size = width * height;
    uint8_t* y_plane = frame_data;
    
    for (int i = 0; i < y_plane_size; i++) {
        int new_value = y_plane[i] + brightness_offset;
        // Clamp to 0-255
        y_plane[i] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
    }
}

// Helper function to adjust contrast of YUV420 frame
void adjust_contrast_yuv420(uint8_t* frame_data, int width, int height, int contrast) {
    // Normalize contrast from 0-255 to 0-2
    float contrast_factor = contrast / 128.0f;
    
    // Only adjust Y plane (luminance)
    int y_plane_size = width * height;
    uint8_t* y_plane = frame_data;
    
    for (int i = 0; i < y_plane_size; i++) {
        // Apply contrast: new_value = 128 + (old_value - 128) * contrast_factor
        int new_value = 128 + static_cast<int>((y_plane[i] - 128) * contrast_factor);
        // Clamp to 0-255
        y_plane[i] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
    }
}

// Helper function to adjust saturation of YUV420 frame
void adjust_saturation_yuv420(uint8_t* frame_data, int width, int height, int saturation) {
    // Normalize saturation from 0-255 to 0-2
    float saturation_factor = saturation / 128.0f;
    
    // Only adjust U and V planes (chrominance)
    int y_plane_size = width * height;
    uint8_t* u_plane = frame_data + y_plane_size;
    uint8_t* v_plane = u_plane + (y_plane_size / 4);
    
    for (int i = 0; i < (y_plane_size / 4); i++) {
        // Apply saturation: new_value = 128 + (old_value - 128) * saturation_factor
        int new_u = 128 + static_cast<int>((u_plane[i] - 128) * saturation_factor);
        int new_v = 128 + static_cast<int>((v_plane[i] - 128) * saturation_factor);
        
        // Clamp to 0-255
        u_plane[i] = static_cast<uint8_t>(std::max(0, std::min(255, new_u)));
        v_plane[i] = static_cast<uint8_t>(std::max(0, std::min(255, new_v)));
    }
}

// Helper function to generate a test pattern for YUV420 format
void generate_test_pattern_yuv420(uint8_t* frame_data, int width, int height, int pattern_type) {
    int y_plane_size = width * height;
    uint8_t* y_plane = frame_data;
    uint8_t* u_plane = y_plane + y_plane_size;
    uint8_t* v_plane = u_plane + (y_plane_size / 4);
    
    // Clear all planes
    std::fill(y_plane, y_plane + y_plane_size, 0);
    std::fill(u_plane, u_plane + (y_plane_size / 4), 128);
    std::fill(v_plane, v_plane + (y_plane_size / 4), 128);
    
    switch (pattern_type) {
        case 0: // Gradient
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    // Y value: gradient from top to bottom
                    y_plane[y * width + x] = static_cast<uint8_t>((y * 255) / height);
                }
            }
            break;
            
        case 1: // Checkerboard
            {
                int cell_size = 32;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        // Checkerboard pattern
                        bool is_white = ((x / cell_size) + (y / cell_size)) % 2 == 0;
                        y_plane[y * width + x] = is_white ? 255 : 0;
                    }
                }
            }
            break;
            
        case 2: // Color bars
            {
                int bar_width = width / 8;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        int bar = x / bar_width;
                        
                        // Y plane values for color bars
                        switch (bar) {
                            case 0: y_plane[y * width + x] = 255; break; // White
                            case 1: y_plane[y * width + x] = 255; break; // Yellow
                            case 2: y_plane[y * width + x] = 180; break; // Cyan
                            case 3: y_plane[y * width + x] = 180; break; // Green
                            case 4: y_plane[y * width + x] = 100; break; // Magenta
                            case 5: y_plane[y * width + x] = 100; break; // Red
                            case 6: y_plane[y * width + x] = 50; break;  // Blue
                            case 7: y_plane[y * width + x] = 0; break;   // Black
                        }
                        
                        // U and V planes for color bars (downsampled)
                        if (x % 2 == 0 && y % 2 == 0) {
                            int uv_index = (y / 2) * (width / 2) + (x / 2);
                            switch (bar) {
                                case 0: u_plane[uv_index] = 128; v_plane[uv_index] = 128; break; // White
                                case 1: u_plane[uv_index] = 0; v_plane[uv_index] = 255; break;   // Yellow
                                case 2: u_plane[uv_index] = 255; v_plane[uv_index] = 0; break;   // Cyan
                                case 3: u_plane[uv_index] = 0; v_plane[uv_index] = 0; break;     // Green
                                case 4: u_plane[uv_index] = 255; v_plane[uv_index] = 255; break; // Magenta
                                case 5: u_plane[uv_index] = 0; v_plane[uv_index] = 255; break;   // Red
                                case 6: u_plane[uv_index] = 255; v_plane[uv_index] = 0; break;   // Blue
                                case 7: u_plane[uv_index] = 128; v_plane[uv_index] = 128; break; // Black
                            }
                        }
                    }
                }
            }
            break;
            
        case 3: // Crosshatch
            {
                int line_spacing = 32;
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        // Crosshatch pattern
                        bool is_line = (x % line_spacing == 0) || (y % line_spacing == 0);
                        y_plane[y * width + x] = is_line ? 255 : 0;
                    }
                }
            }
            break;
    }
}

// Helper function to convert between YUV formats
void convert_yuv_format(uint8_t* src_data, uint8_t* dst_data, int width, int height, 
                        int src_format, int dst_format) {
    // This is a simplified implementation that only handles a few common conversions
    
    if (src_format == dst_format) {
        // No conversion needed, just copy the data
        int src_size = calculate_frame_size(src_format, width, height);
        std::memcpy(dst_data, src_data, src_size);
        return;
    }
    
    if (src_format == SCE_CAMERA_FORMAT_YUV420 && dst_format == SCE_CAMERA_FORMAT_YUV422) {
        // Convert YUV420 to YUV422
        int y_plane_size = width * height;
        uint8_t* src_y = src_data;
        uint8_t* src_u = src_y + y_plane_size;
        uint8_t* src_v = src_u + (y_plane_size / 4);
        
        // YUV422 interleaved: Y0 U0 Y1 V0 Y2 U1 Y3 V1 ...
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x += 2) {
                int src_y_pos = y * width + x;
                int src_uv_pos = (y / 2) * (width / 2) + (x / 2);
                int dst_pos = (y * width + x) * 2;
                
                // Y0 U0 Y1 V0
                dst_data[dst_pos] = src_y[src_y_pos];
                dst_data[dst_pos + 1] = src_u[src_uv_pos];
                dst_data[dst_pos + 2] = src_y[src_y_pos + 1];
                dst_data[dst_pos + 3] = src_v[src_uv_pos];
            }
        }
    } else {
        // For other conversions, just fill with a pattern for now
        // In a real implementation, proper conversion would be needed
        generate_test_pattern_yuv420(dst_data, width, height, 0);
    }
}

} // namespace camera
} // namespace modules
} // namespace firmware
