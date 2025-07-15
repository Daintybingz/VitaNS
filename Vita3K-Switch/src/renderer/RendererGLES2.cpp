#include "RendererGLES2.h"
#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <tuple>

namespace {
const char* vertex_shader_src = R"(
attribute vec2 aPos;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vTexCoord = aTexCoord;
}
)";

const char* fragment_shader_src = R"(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uTexture;
void main() {
    gl_FragColor = texture2D(uTexture, vTexCoord);
}
)";
}

bool RendererGLES2::create_context() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    window = SDL_CreateWindow("VitaNS GLES2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        return false;
    }
    SDL_GL_MakeCurrent(window, gl_context);
    return true;
}

void RendererGLES2::destroy_context() {
    if (gl_context) {
        SDL_GL_DeleteContext(gl_context);
        gl_context = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

GLuint compile_shader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool RendererGLES2::create_shader() {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    if (!vs || !fs) return false;
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glBindAttribLocation(program, 0, "aPos");
    glBindAttribLocation(program, 1, "aTexCoord");
    glLinkProgram(program);
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!status) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;
        glDeleteProgram(program);
        program = 0;
        return false;
    }
    return true;
}

void RendererGLES2::destroy_shader() {
    if (program) {
        glDeleteProgram(program);
        program = 0;
    }
}

void RendererGLES2::create_texture(int w, int h) {
    if (texture) {
        glDeleteTextures(1, &texture);
        texture = 0;
    }
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    tex_width = w;
    tex_height = h;
}

void RendererGLES2::upload_framebuffer(const uint8_t* pixels, int w, int h) {
    if (!texture || tex_width != w || tex_height != h) {
        create_texture(w, h);
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool RendererGLES2::init() {
    if (!create_context()) return false;
    if (!create_shader()) return false;
    // Setup a simple fullscreen quad VBO (two triangles)
    float vertices[] = {
        // pos      // tex
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

void RendererGLES2::draw_frame(/* const FrameData& frame */) {
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    if (texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (texture) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void RendererGLES2::present() {
    SDL_GL_SwapWindow(window);
}

void RendererGLES2::shutdown() {
    destroy_shader();
    destroy_context();
}

RendererGLES2::~RendererGLES2() {
    shutdown();
}

void RendererGLES2::clear(uint32_t mask) {
    // Map mask bits to GL buffers (assume 0x1 = color, 0x2 = depth, 0x4 = stencil)
    GLbitfield glMask = 0;
    if (mask & 0x1) glMask |= GL_COLOR_BUFFER_BIT;
    if (mask & 0x2) glMask |= GL_DEPTH_BUFFER_BIT;
    if (mask & 0x4) glMask |= GL_STENCIL_BUFFER_BIT;
    glClear(glMask);
}

void RendererGLES2::setState(uint32_t stateId, uint32_t value) {
    // State IDs: 0=blend enable, 1=depth test enable, 2=stencil test enable, 3=blend mode, 4=blend src, 5=blend dst, 6=depth func, 7=stencil func
    switch (stateId) {
        case 0: // Blend enable
            if (value) glEnable(GL_BLEND); else glDisable(GL_BLEND);
            break;
        case 1: // Depth test enable
            if (value) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
            break;
        case 2: // Stencil test enable
            if (value) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);
            break;
        case 3: // Blend mode (func)
            switch (value) {
                case 0: glBlendEquation(GL_FUNC_ADD); break;
                case 1: glBlendEquation(GL_FUNC_ADD); break;
                case 2: glBlendEquation(GL_FUNC_SUBTRACT); break;
                case 3: glBlendEquation(GL_FUNC_REVERSE_SUBTRACT); break;
                case 4: glBlendEquation(GL_MIN); break;
                case 5: glBlendEquation(GL_MAX); break;
                default: glBlendEquation(GL_FUNC_ADD); break;
            }
            break;
        case 4: // Blend src factor
            glBlendFuncSeparate(value, GL_ONE_MINUS_SRC_ALPHA, value, GL_ONE_MINUS_SRC_ALPHA); // Simplified
            break;
        case 5: // Blend dst factor
            glBlendFuncSeparate(GL_SRC_ALPHA, value, GL_SRC_ALPHA, value); // Simplified
            break;
        case 6: // Depth func
            switch (value) {
                case 0: glDepthFunc(GL_NEVER); break;
                case 1: glDepthFunc(GL_LESS); break;
                case 2: glDepthFunc(GL_EQUAL); break;
                case 3: glDepthFunc(GL_LEQUAL); break;
                case 4: glDepthFunc(GL_GREATER); break;
                case 5: glDepthFunc(GL_NOTEQUAL); break;
                case 6: glDepthFunc(GL_GEQUAL); break;
                case 7: glDepthFunc(GL_ALWAYS); break;
                default: glDepthFunc(GL_LESS); break;
            }
            break;
        case 7: // Stencil func
            glStencilFunc(value, 0, 0xFF); // Simplified
            break;
        default:
            break;
    }
}

void RendererGLES2::setupVertexAttributes(const std::vector<std::pair<int, int>>& attribs) {
    // attribs: vector of (index, size) pairs, e.g. (0,3)=vec3 position, (1,2)=vec2 texcoord
    for (const auto& attr : attribs) {
        glEnableVertexAttribArray(attr.first);
        glVertexAttribPointer(attr.first, attr.second, GL_FLOAT, GL_FALSE, 0, (void*)0); // Simplified
    }
}

void RendererGLES2::setupVertexAttributesDynamic(const std::vector<std::tuple<int, int, GLenum, size_t>>& layout, size_t stride) {
    for (const auto& attr : layout) {
        int index = std::get<0>(attr);
        int size = std::get<1>(attr);
        GLenum type = std::get<2>(attr);
        size_t offset = std::get<3>(attr);
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, type, GL_FALSE, stride, (const void*)offset);
    }
}

void RendererGLES2::setUniform(const std::string& name, float value) {
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    GLint loc = glGetUniformLocation(prog, name.c_str());
    if (loc >= 0) glUniform1f(loc, value);
}
void RendererGLES2::setUniformVec(const std::string& name, const float* values, int count) {
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    GLint loc = glGetUniformLocation(prog, name.c_str());
    if (loc >= 0) {
        if (count == 2) glUniform2fv(loc, 1, values);
        else if (count == 3) glUniform3fv(loc, 1, values);
        else if (count == 4) glUniform4fv(loc, 1, values);
    }
}
void RendererGLES2::setSampler(const std::string& name, int unit) {
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    GLint loc = glGetUniformLocation(prog, name.c_str());
    if (loc >= 0) glUniform1i(loc, unit);
}

void RendererGLES2::draw(uint32_t primType, uint32_t indexType, const void* indexData, uint32_t indexCount) {
    // Map primType to GL primitive
    GLenum glPrim = GL_TRIANGLES;
    switch (primType) {
        case 0: glPrim = GL_TRIANGLES; break;
        case 1: glPrim = GL_TRIANGLE_STRIP; break;
        case 2: glPrim = GL_TRIANGLE_FAN; break;
        case 3: glPrim = GL_LINES; break;
        case 4: glPrim = GL_LINE_STRIP; break;
        case 5: glPrim = GL_POINTS; break;
        default: glPrim = GL_TRIANGLES; break;
    }
    // Use the selected shader if set
    if (!currentShaderName.empty() && shaders.count(currentShaderName)) {
        glUseProgram(shaders[currentShaderName]);
    } else if (currentShaderId && shaders.count(std::to_string(currentShaderId))) {
        glUseProgram(shaders[std::to_string(currentShaderId)]);
    } else if (!shaders.empty()) {
        glUseProgram(shaders.begin()->second);
    } else {
        glUseProgram(program);
    }
    // Bind uploaded VBO/IBO if available
    if (currentVBO) glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
    if (currentIBO) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIBO);
    // Example: setup attributes for position (vec3) and texcoord (vec2)
    std::vector<std::pair<int, int>> attribs = { {0, 3}, {1, 2} };
    setupVertexAttributes(attribs);
    if (texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(program, "uTexture"), 0);
    }
    if (currentIBO && indexCount > 0) {
        GLenum glIndexType = (currentIndexType == 0) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        glDrawElements(glPrim, indexCount, glIndexType, 0);
    } else {
        glDrawArrays(glPrim, 0, indexCount > 0 ? indexCount : 6);
    }
    if (texture) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    if (currentVBO) glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (currentIBO) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
}

void RendererGLES2::bindTexture(uint32_t textureUnit, uint32_t textureId) {
    // For now, just bind the main framebuffer texture
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture ? texture : 0);
}

void RendererGLES2::uploadTexture(uint32_t textureId, const void* data, int w, int h, int format) {
    GLuint tex = 0;
    if (textures.count(textureId)) {
        tex = textures[textureId];
    } else {
        glGenTextures(1, &tex);
        textures[textureId] = tex;
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLenum glFormat = (format == 0) ? GL_RGBA : GL_RGB; // Simplified
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, w, h, 0, glFormat, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RendererGLES2::useTexture(uint32_t textureUnit, uint32_t textureId) {
    if (textures.count(textureId)) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, textures[textureId]);
    }
}

void RendererGLES2::createShaderFromSource(const std::string& name, const std::string& vertSrc, const std::string& fragSrc) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragSrc.c_str());
    if (!vs || !fs) {
        printf("[RendererGLES2] Shader compile error for %s\n", name.c_str());
        return;
    }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!status) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        printf("[RendererGLES2] Program link error for %s: %s\n", name.c_str(), log);
        glDeleteProgram(prog);
        return;
    }
    shaders[name] = prog;
}

void RendererGLES2::useShader(const std::string& name) {
    if (shaders.count(name)) {
        glUseProgram(shaders[name]);
    }
}

void RendererGLES2::uploadVertexBuffer(const void* data, size_t size) {
    if (!currentVBO) glGenBuffers(1, &currentVBO);
    glBindBuffer(GL_ARRAY_BUFFER, currentVBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) printf("[RendererGLES2] Vertex buffer upload error: %x\n", err);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RendererGLES2::uploadIndexBuffer(const void* data, size_t size, uint32_t indexType) {
    if (!currentIBO) glGenBuffers(1, &currentIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    currentIndexType = indexType;
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) printf("[RendererGLES2] Index buffer upload error: %x\n", err);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RendererGLES2::setCurrentShader(const std::string& name) {
    currentShaderName = name;
}
void RendererGLES2::setCurrentShaderById(uint32_t id) {
    currentShaderId = id;
}

void RendererGLES2::cleanupResources() {
    // Delete all GL buffers and shaders
    for (auto& kv : textures) glDeleteTextures(1, &kv.second);
    textures.clear();
    for (auto& kv : shaders) glDeleteProgram(kv.second);
    shaders.clear();
    if (currentVBO) { glDeleteBuffers(1, &currentVBO); currentVBO = 0; }
    if (currentIBO) { glDeleteBuffers(1, &currentIBO); currentIBO = 0; }
} 