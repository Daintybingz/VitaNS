#include "include/framebuffer_manager.h"
#include <iostream>
#include <sstream>

namespace renderer::gl {

bool FramebufferManager::scale_framebuffer(uint32_t handle, uint32_t target_width, uint32_t target_height) {
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    auto it = framebuffers.find(handle);
    if (it == framebuffers.end()) {
        return false;
    }
    
    Framebuffer& fb = it->second;
    
    // Create a new framebuffer for the scaled result if needed
    if (fb.scaled_width != target_width || fb.scaled_height != target_height) {
        // Create or resize the scaled framebuffer
        GLuint scaled_fbo;
        GLuint scaled_texture;
        
        glGenFramebuffers(1, &scaled_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, scaled_fbo);
        
        glGenTextures(1, &scaled_texture);
        glBindTexture(GL_TEXTURE_2D, scaled_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, target_width, target_height, 0, 
                    GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scaled_texture, 0);
        
        // Use the appropriate scaling shader
        GLuint scaling_shader = scaling_shaders[fb.scaling];
        if (scaling_shader == 0) {
            scaling_shader = create_scaling_shader(fb.scaling);
            scaling_shaders[fb.scaling] = scaling_shader;
        }
        
        // Render the original framebuffer to the scaled framebuffer using the scaling shader
        glUseProgram(scaling_shader);
        
        // Set uniforms for the scaling shader
        GLint source_texture_loc = glGetUniformLocation(scaling_shader, "sourceTexture");
        GLint source_size_loc = glGetUniformLocation(scaling_shader, "sourceSize");
        GLint target_size_loc = glGetUniformLocation(scaling_shader, "targetSize");
        
        glUniform1i(source_texture_loc, 0);
        glUniform2f(source_size_loc, static_cast<float>(fb.width), static_cast<float>(fb.height));
        glUniform2f(target_size_loc, static_cast<float>(target_width), static_cast<float>(target_height));
        
        // Bind the source texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fb.color_texture);
        
        // Render full-screen quad
        glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        // Update the framebuffer with the scaled texture
        if (fb.effect_fbo != 0) {
            // If we have an effect framebuffer, update it
            glDeleteTextures(1, &fb.effect_texture);
            fb.effect_texture = scaled_texture;
        } else {
            // Otherwise update the main framebuffer
            glDeleteTextures(1, &fb.color_texture);
            fb.color_texture = scaled_texture;
        }
        
        // Clean up the temporary FBO
        glDeleteFramebuffers(1, &scaled_fbo);
        
        // Update dimensions
        fb.scaled_width = target_width;
        fb.scaled_height = target_height;
        
        std::cout << "[Renderer] Scaled framebuffer " << handle << " from " << fb.width << "x" << fb.height 
                  << " to " << target_width << "x" << target_height << std::endl;
    }
    
    return true;
}

GLuint FramebufferManager::create_scaling_shader(ScalingMode mode) {
    // Create shader program
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Common vertex shader for all scaling modes
    const char* vertex_shader_source = R"(#version 320 es
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texCoord;
        out vec2 v_texCoord;
        
        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
            v_texCoord = texCoord;
        }
    )";
    
    // Fragment shader depends on the scaling mode
    std::string fragment_shader_source;
    
    switch (mode) {
        case ScalingMode::None:
        case ScalingMode::Bilinear:
            // Simple bilinear filtering (handled by GPU)
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 sourceSize;
                uniform vec2 targetSize;
                out vec4 fragColor;
                
                void main() {
                    // Simple bilinear filtering is handled by the GPU
                    fragColor = texture(sourceTexture, v_texCoord);
                }
            )";
            break;
            
        case ScalingMode::Bicubic:
            // Bicubic filtering for better quality
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 sourceSize;
                uniform vec2 targetSize;
                out vec4 fragColor;
                
                // Cubic weight function
                float cubic(float x) {
                    float a = -0.5;
                    
                    if (x < 0.0) x = -x;
                    
                    float x2 = x * x;
                    float x3 = x2 * x;
                    
                    if (x <= 1.0) {
                        return (a + 2.0) * x3 - (a + 3.0) * x2 + 1.0;
                    } else if (x <= 2.0) {
                        return a * x3 - 5.0 * a * x2 + 8.0 * a * x - 4.0 * a;
                    } else {
                        return 0.0;
                    }
                }
                
                void main() {
                    vec2 texel_size = 1.0 / sourceSize;
                    vec2 tex_coord = v_texCoord * sourceSize;
                    vec2 tex_coord_fract = fract(tex_coord);
                    tex_coord = floor(tex_coord) * texel_size;
                    
                    vec4 color = vec4(0.0);
                    float total_weight = 0.0;
                    
                    for (int y = -1; y <= 2; y++) {
                        for (int x = -1; x <= 2; x++) {
                            vec2 offset = vec2(float(x), float(y));
                            vec2 sample_pos = tex_coord + offset * texel_size;
                            
                            float weight = cubic(offset.x - tex_coord_fract.x) * cubic(offset.y - tex_coord_fract.y);
                            color += texture(sourceTexture, sample_pos) * weight;
                            total_weight += weight;
                        }
                    }
                    
                    fragColor = color / total_weight;
                }
            )";
            break;
            
        case ScalingMode::FSR:
            // AMD FidelityFX Super Resolution (simplified version)
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 sourceSize;
                uniform vec2 targetSize;
                out vec4 fragColor;
                
                // Simplified FSR implementation
                // In a real implementation, this would be much more complex
                
                // EASU (Edge Adaptive Spatial Upsampling) step
                vec4 easu(vec2 uv) {
                    // Simplified edge detection and adaptive upsampling
                    vec2 texel_size = 1.0 / sourceSize;
                    
                    vec4 center = texture(sourceTexture, uv);
                    vec4 left = texture(sourceTexture, uv + vec2(-texel_size.x, 0.0));
                    vec4 right = texture(sourceTexture, uv + vec2(texel_size.x, 0.0));
                    vec4 top = texture(sourceTexture, uv + vec2(0.0, texel_size.y));
                    vec4 bottom = texture(sourceTexture, uv + vec2(0.0, -texel_size.y));
                    
                    // Edge detection
                    float edge_h = abs(left.g - center.g) + abs(right.g - center.g);
                    float edge_v = abs(top.g - center.g) + abs(bottom.g - center.g);
                    
                    // Adaptive weight
                    float weight_h = 1.0 - smoothstep(0.0, 0.2, edge_h);
                    float weight_v = 1.0 - smoothstep(0.0, 0.2, edge_v);
                    
                    // Weighted average
                    return center;
                }
                
                // RCAS (Robust Contrast Adaptive Sharpening) step
                vec4 rcas(vec4 color) {
                    // Simplified contrast adaptive sharpening
                    float sharpness = 0.5; // Adjustable
                    
                    // Apply sharpening
                    vec3 luma = vec3(0.299, 0.587, 0.114);
                    float center_luma = dot(color.rgb, luma);
                    
                    vec2 texel_size = 1.0 / targetSize;
                    vec4 left = texture(sourceTexture, v_texCoord + vec2(-texel_size.x, 0.0));
                    vec4 right = texture(sourceTexture, v_texCoord + vec2(texel_size.x, 0.0));
                    vec4 top = texture(sourceTexture, v_texCoord + vec2(0.0, texel_size.y));
                    vec4 bottom = texture(sourceTexture, v_texCoord + vec2(0.0, -texel_size.y));
                    
                    float left_luma = dot(left.rgb, luma);
                    float right_luma = dot(right.rgb, luma);
                    float top_luma = dot(top.rgb, luma);
                    float bottom_luma = dot(bottom.rgb, luma);
                    
                    float min_luma = min(center_luma, min(min(left_luma, right_luma), min(top_luma, bottom_luma)));
                    float max_luma = max(center_luma, max(max(left_luma, right_luma), max(top_luma, bottom_luma)));
                    
                    // Sharpen within dynamic range
                    float sharpen_amount = sharpness * (1.0 - (max_luma - min_luma));
                    
                    vec4 sharpened = color + sharpen_amount * (color - (left + right + top + bottom) * 0.25);
                    
                    return sharpened;
                }
                
                void main() {
                    // Apply EASU (upscaling)
                    vec4 upscaled = easu(v_texCoord);
                    
                    // Apply RCAS (sharpening)
                    fragColor = rcas(upscaled);
                }
            )";
            break;
            
        case ScalingMode::Custom:
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 sourceSize;
                uniform vec2 targetSize;
                out vec4 fragColor;
                void main() {
                    fragColor = texture(sourceTexture, v_texCoord);
                }
            )";
            break;
    }
    
    // Compile shaders
    const char* vs_source = vertex_shader_source;
    const char* fs_source = fragment_shader_source.c_str();
    
    glShaderSource(vertex_shader, 1, &vs_source, nullptr);
    glCompileShader(vertex_shader);
    
    glShaderSource(fragment_shader, 1, &fs_source, nullptr);
    glCompileShader(fragment_shader);
    
    // Check compilation status
    GLint success;
    GLchar info_log[512];
    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
        std::cerr << "[Renderer] Vertex shader compilation failed: " << info_log << std::endl;
        return 0;
    }
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
        std::cerr << "[Renderer] Fragment shader compilation failed: " << info_log << std::endl;
        return 0;
    }
    
    // Create and link shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    // Check linking status
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
        std::cerr << "[Renderer] Shader program linking failed: " << info_log << std::endl;
        return 0;
    }
    
    // Clean up
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    std::cout << "[Renderer] Created scaling shader for mode " << static_cast<int>(mode) << std::endl;
    
    return program;
}

} // namespace renderer::gl
