#include "display_buffer.h"
#include "../renderer/gl/switch_renderer.h"
#include "../core/memory/memory_manager.h"
#include <cstdio>
#include <cstring>

// Simple vertex shader for rendering the framebuffer texture
const char* vertexShaderSource = R"(
    #version 100
    attribute vec2 position;
    attribute vec2 texCoord;
    varying vec2 fragTexCoord;
    void main() {
        gl_Position = vec4(position, 0.0, 1.0);
        fragTexCoord = texCoord;
    }
)";

// Simple fragment shader for rendering the framebuffer texture
const char* fragmentShaderSource = R"(
    #version 100
    precision mediump float;
    varying vec2 fragTexCoord;
    uniform sampler2D texture;
    void main() {
        gl_FragColor = texture2D(texture, fragTexCoord);
    }
)";

DisplayBuffer::DisplayBuffer()
    : renderer(nullptr), initialized(false),
      textureId(0), vertexBuffer(0), indexBuffer(0), shaderProgram(0) {
    // Initialize the current buffer to default values
    memset(&currentBuffer, 0, sizeof(DisplayFrameBuffer));
    currentBuffer.width = VITA_DISPLAY_WIDTH;
    currentBuffer.height = VITA_DISPLAY_HEIGHT;
    currentBuffer.stride = VITA_DISPLAY_STRIDE;
    currentBuffer.pixelFormat = VITA_DISPLAY_PIXEL_FORMAT_A8B8G8R8;
}

DisplayBuffer::~DisplayBuffer() {
    finalize();
}

bool DisplayBuffer::initialize(SwitchRenderer* renderer) {
    if (initialized) {
        return true;
    }
    
    if (!renderer) {
        printf("[DisplayBuffer] Cannot initialize: renderer is null\n");
        return false;
    }
    
    this->renderer = renderer;
    
    // Initialize OpenGL resources
    if (!initializeGL()) {
        printf("[DisplayBuffer] Failed to initialize OpenGL resources\n");
        return false;
    }
    
    initialized = true;
    printf("[DisplayBuffer] Initialized successfully\n");
    return true;
}

void DisplayBuffer::finalize() {
    if (!initialized) {
        return;
    }
    
    // Clean up OpenGL resources
    if (textureId) {
        glDeleteTextures(1, &textureId);
        textureId = 0;
    }
    
    if (vertexBuffer) {
        glDeleteBuffers(1, &vertexBuffer);
        vertexBuffer = 0;
    }
    
    if (indexBuffer) {
        glDeleteBuffers(1, &indexBuffer);
        indexBuffer = 0;
    }
    
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
    
    renderer = nullptr;
    initialized = false;
    printf("[DisplayBuffer] Finalized\n");
}

bool DisplayBuffer::setFrameBuffer(uint32_t addr, uint32_t width, uint32_t height, uint32_t stride, uint32_t pixelFormat) {
    if (!initialized) {
        printf("[DisplayBuffer] Cannot set framebuffer: not initialized\n");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Update the current buffer
    currentBuffer.addr = addr;
    currentBuffer.width = width;
    currentBuffer.height = height;
    currentBuffer.stride = stride;
    currentBuffer.pixelFormat = pixelFormat;
    
    printf("[DisplayBuffer] Framebuffer set: addr=0x%08X, width=%u, height=%u, stride=%u, format=%u\n",
           addr, width, height, stride, pixelFormat);
    
    return true;
}

bool DisplayBuffer::getFrameBuffer(uint32_t* addr, uint32_t* width, uint32_t* height, uint32_t* stride, uint32_t* pixelFormat) {
    if (!initialized) {
        printf("[DisplayBuffer] Cannot get framebuffer: not initialized\n");
        return false;
    }
    
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Copy the current buffer info to the output parameters
    if (addr) *addr = currentBuffer.addr;
    if (width) *width = currentBuffer.width;
    if (height) *height = currentBuffer.height;
    if (stride) *stride = currentBuffer.stride;
    if (pixelFormat) *pixelFormat = currentBuffer.pixelFormat;
    
    return true;
}

void DisplayBuffer::render() {
    if (!initialized || !renderer) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(bufferMutex);
    
    // Begin rendering
    renderer->beginFrame();
    
    // Update texture from PS Vita memory
    updateTexture();
    
    // Render the framebuffer texture to the screen
    glUseProgram(shaderProgram);
    
    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set the texture uniform
    GLint textureUniform = glGetUniformLocation(shaderProgram, "texture");
    glUniform1i(textureUniform, 0);
    
    // Bind the vertex and index buffers
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    
    // Set up vertex attributes
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    GLint texCoordAttrib = glGetAttribLocation(shaderProgram, "texCoord");
    glEnableVertexAttribArray(texCoordAttrib);
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    // Draw the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    
    // Clean up
    glDisableVertexAttribArray(posAttrib);
    glDisableVertexAttribArray(texCoordAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    // End rendering
    renderer->endFrame();
    
    // Swap buffers
    renderer->swapBuffers();
}

void DisplayBuffer::waitVblank() {
    // In a real implementation, this would wait for the vertical blank period
    // For now, we'll just simulate it with a small delay
    SDL_Delay(16); // ~60 FPS
}

bool DisplayBuffer::initializeGL() {
    // Create and bind the texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate texture storage
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VITA_DISPLAY_WIDTH, VITA_DISPLAY_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Create vertex buffer (position and texture coordinates)
    float vertices[] = {
        // Position (x, y), TexCoord (u, v)
        -1.0f, -1.0f, 0.0f, 1.0f, // Bottom-left
         1.0f, -1.0f, 1.0f, 1.0f, // Bottom-right
         1.0f,  1.0f, 1.0f, 0.0f, // Top-right
        -1.0f,  1.0f, 0.0f, 0.0f  // Top-left
    };
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Create index buffer
    uint16_t indices[] = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };
    
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // Create shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check for shader compilation and linking errors
    GLint success;
    GLchar infoLog[512];
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        printf("[DisplayBuffer] Vertex shader compilation failed: %s\n", infoLog);
        return false;
    }
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        printf("[DisplayBuffer] Fragment shader compilation failed: %s\n", infoLog);
        return false;
    }
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        printf("[DisplayBuffer] Shader program linking failed: %s\n", infoLog);
        return false;
    }
    
    // Clean up shaders (they're linked to the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

void DisplayBuffer::updateTexture() {
    if (!currentBuffer.addr || !currentBuffer.width || !currentBuffer.height) {
        return;
    }
    
    // In a real implementation, this would copy data from PS Vita memory to the texture
    // For now, we'll just update the texture with a test pattern
    
    // Create a test pattern (checkerboard)
    uint32_t* pixels = new uint32_t[currentBuffer.width * currentBuffer.height];
    
    for (uint32_t y = 0; y < currentBuffer.height; y++) {
        for (uint32_t x = 0; x < currentBuffer.width; x++) {
            uint32_t color;
            if ((x / 32 + y / 32) % 2 == 0) {
                color = 0xFF0000FF; // Red
            } else {
                color = 0xFF00FF00; // Green
            }
            pixels[y * currentBuffer.width + x] = color;
        }
    }
    
    // Update the texture
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, currentBuffer.width, currentBuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    delete[] pixels;
}
