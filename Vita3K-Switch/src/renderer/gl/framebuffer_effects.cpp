#include "include/framebuffer_manager.h"
#include <iostream>
#include <sstream>

namespace renderer::gl {

bool FramebufferManager::create_effect_framebuffer(Framebuffer& fb) {
    // Create a framebuffer for post-processing effects
    glGenFramebuffers(1, &fb.effect_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.effect_fbo);
    
    // Create a texture for the effect framebuffer
    glGenTextures(1, &fb.effect_texture);
    glBindTexture(GL_TEXTURE_2D, fb.effect_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb.scaled_width, fb.scaled_height, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.effect_texture, 0);
    
    // Check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[Renderer] Effect framebuffer creation failed: " << status << std::endl;
        return false;
    }
    
    return true;
}

bool FramebufferManager::apply_effects(uint32_t handle) {
    if (!effects_enabled) {
        return true;
    }
    
    std::lock_guard<std::mutex> lock(fb_mutex);
    
    auto it = framebuffers.find(handle);
    if (it == framebuffers.end()) {
        return false;
    }
    
    Framebuffer& fb = it->second;
    
    // Skip if no effect is set
    if (fb.effect == FramebufferEffect::None) {
        return true;
    }
    
    // Apply the effect
    return apply_effect(fb, fb.effect);
}

bool FramebufferManager::apply_effect(Framebuffer& fb, FramebufferEffect effect) {
    // Create effect framebuffer if it doesn't exist
    if (fb.effect_fbo == 0) {
        if (!create_effect_framebuffer(fb)) {
            return false;
        }
    }
    
    // Get or create the effect shader
    GLuint effect_shader = effect_shaders[effect];
    if (effect_shader == 0) {
        effect_shader = create_effect_shader(effect);
        effect_shaders[effect] = effect_shader;
    }
    
    // Bind the effect framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fb.effect_fbo);
    
    // Use the effect shader
    glUseProgram(effect_shader);
    
    // Set uniforms for the effect shader
    GLint source_texture_loc = glGetUniformLocation(effect_shader, "sourceTexture");
    GLint resolution_loc = glGetUniformLocation(effect_shader, "resolution");
    
    glUniform1i(source_texture_loc, 0);
    glUniform2f(resolution_loc, static_cast<float>(fb.scaled_width), static_cast<float>(fb.scaled_height));
    
    // Add effect-specific uniforms
    switch (effect) {
        case FramebufferEffect::Blur:
            {
                GLint blur_radius_loc = glGetUniformLocation(effect_shader, "blurRadius");
                glUniform1f(blur_radius_loc, 2.0f); // Configurable blur radius
            }
            break;
        case FramebufferEffect::ColorCorrection:
            {
                GLint brightness_loc = glGetUniformLocation(effect_shader, "brightness");
                GLint contrast_loc = glGetUniformLocation(effect_shader, "contrast");
                GLint saturation_loc = glGetUniformLocation(effect_shader, "saturation");
                
                glUniform1f(brightness_loc, 1.0f); // Configurable brightness
                glUniform1f(contrast_loc, 1.0f);   // Configurable contrast
                glUniform1f(saturation_loc, 1.0f); // Configurable saturation
            }
            break;
        case FramebufferEffect::Bloom:
            {
                GLint threshold_loc = glGetUniformLocation(effect_shader, "threshold");
                GLint intensity_loc = glGetUniformLocation(effect_shader, "intensity");
                
                glUniform1f(threshold_loc, 0.8f);  // Configurable threshold
                glUniform1f(intensity_loc, 0.5f);  // Configurable intensity
            }
            break;
        default:
            break;
    }
    
    // Bind the source texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb.color_texture);
    
    // Render full-screen quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // Swap the textures so the effect result becomes the new framebuffer texture
    std::swap(fb.color_texture, fb.effect_texture);
    
    return true;
}

GLuint FramebufferManager::create_effect_shader(FramebufferEffect effect) {
    // Create shader program
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Common vertex shader for all effects
    const char* vertex_shader_source = R"(#version 320 es
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 texCoord;
        out vec2 v_texCoord;
        
        void main() {
            gl_Position = vec4(position, 0.0, 1.0);
            v_texCoord = texCoord;
        }
    )";
    
    // Fragment shader depends on the effect
    std::string fragment_shader_source;
    
    switch (effect) {
        case FramebufferEffect::Blur:
            // Gaussian blur effect
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 resolution;
                uniform float blurRadius;
                out vec4 fragColor;
                
                void main() {
                    vec4 color = vec4(0.0);
                    float total_weight = 0.0;
                    
                    // Two-pass Gaussian blur (this is the horizontal pass)
                    float sigma = blurRadius / 2.0;
                    float sigma_squared = sigma * sigma;
                    
                    for (float x = -blurRadius; x <= blurRadius; x += 1.0) {
                        float weight = exp(-(x * x) / (2.0 * sigma_squared));
                        vec2 offset = vec2(x / resolution.x, 0.0);
                        color += texture(sourceTexture, v_texCoord + offset) * weight;
                        total_weight += weight;
                    }
                    
                    fragColor = color / total_weight;
                }
            )";
            break;
            
        case FramebufferEffect::ColorCorrection:
            // Color correction effect
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 resolution;
                uniform float brightness;
                uniform float contrast;
                uniform float saturation;
                out vec4 fragColor;
                
                // RGB to HSL conversion
                vec3 rgb_to_hsl(vec3 rgb) {
                    float r = rgb.r;
                    float g = rgb.g;
                    float b = rgb.b;
                    
                    float max_val = max(max(r, g), b);
                    float min_val = min(min(r, g), b);
                    float delta = max_val - min_val;
                    
                    float h = 0.0;
                    float s = 0.0;
                    float l = (max_val + min_val) / 2.0;
                    
                    if (delta > 0.0) {
                        s = (l < 0.5) ? (delta / (max_val + min_val)) : (delta / (2.0 - max_val - min_val));
                        
                        if (max_val == r) {
                            h = (g - b) / delta + (g < b ? 6.0 : 0.0);
                        } else if (max_val == g) {
                            h = (b - r) / delta + 2.0;
                        } else {
                            h = (r - g) / delta + 4.0;
                        }
                        
                        h /= 6.0;
                    }
                    
                    return vec3(h, s, l);
                }
                
                // HSL to RGB conversion
                vec3 hsl_to_rgb(vec3 hsl) {
                    float h = hsl.x;
                    float s = hsl.y;
                    float l = hsl.z;
                    
                    vec3 rgb;
                    
                    if (s == 0.0) {
                        rgb = vec3(l);
                    } else {
                        float q = (l < 0.5) ? (l * (1.0 + s)) : (l + s - l * s);
                        float p = 2.0 * l - q;
                        
                        float r = hue_to_rgb(p, q, h + 1.0/3.0);
                        float g = hue_to_rgb(p, q, h);
                        float b = hue_to_rgb(p, q, h - 1.0/3.0);
                        
                        rgb = vec3(r, g, b);
                    }
                    
                    return rgb;
                }
                
                float hue_to_rgb(float p, float q, float t) {
                    if (t < 0.0) t += 1.0;
                    if (t > 1.0) t -= 1.0;
                    
                    if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
                    if (t < 1.0/2.0) return q;
                    if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
                    
                    return p;
                }
                
                void main() {
                    // Get the original color
                    vec4 color = texture(sourceTexture, v_texCoord);
                    
                    // Apply brightness adjustment
                    color.rgb *= brightness;
                    
                    // Apply contrast adjustment
                    color.rgb = (color.rgb - 0.5) * contrast + 0.5;
                    
                    // Apply saturation adjustment
                    vec3 hsl = rgb_to_hsl(color.rgb);
                    hsl.y *= saturation;
                    color.rgb = hsl_to_rgb(hsl);
                    
                    // Clamp to valid range
                    color = clamp(color, 0.0, 1.0);
                    
                    fragColor = color;
                }
            )";
            break;
            
        case FramebufferEffect::Bloom:
            // Bloom effect
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 resolution;
                uniform float threshold;
                uniform float intensity;
                out vec4 fragColor;
                
                // Extract bright areas
                vec4 extract_bright(vec4 color) {
                    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
                    return (brightness > threshold) ? color : vec4(0.0);
                }
                
                // Gaussian blur (simplified)
                vec4 blur(sampler2D tex, vec2 uv, vec2 res) {
                    float blur_size = 4.0;
                    vec4 color = vec4(0.0);
                    float total_weight = 0.0;
                    
                    for (float y = -blur_size; y <= blur_size; y += 1.0) {
                        for (float x = -blur_size; x <= blur_size; x += 1.0) {
                            vec2 offset = vec2(x, y) / res;
                            float weight = exp(-(x*x + y*y) / (2.0 * blur_size * blur_size));
                            color += texture(tex, uv + offset) * weight;
                            total_weight += weight;
                        }
                    }
                    
                    return color / total_weight;
                }
                
                void main() {
                    // Get the original color
                    vec4 original = texture(sourceTexture, v_texCoord);
                    
                    // Extract bright areas
                    vec4 bright = extract_bright(original);
                    
                    // Blur the bright areas
                    vec4 blurred = blur(sourceTexture, v_texCoord, resolution);
                    
                    // Combine original and bloom
                    fragColor = original + blurred * intensity;
                }
            )";
            break;
            
        case FramebufferEffect::Custom:
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
                uniform vec2 resolution;
                out vec4 fragColor;
                void main() {
                    fragColor = texture(sourceTexture, v_texCoord);
                }
            )";
            break;
            
        default:
            // No effect
            fragment_shader_source = R"(#version 320 es
                precision highp float;
                in vec2 v_texCoord;
                uniform sampler2D sourceTexture;
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
    
    std::cout << "[Renderer] Created effect shader for effect " << static_cast<int>(effect) << std::endl;
    
    return program;
}

} // namespace renderer::gl
