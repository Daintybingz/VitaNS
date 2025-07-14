#pragma once
#include "Renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#include <vector>
#include <string>
#include <unordered_map>

class RendererGLES2 : public Renderer {
public:
    bool init() override;
    void draw_frame(/* const FrameData& frame */) override;
    void present() override;
    void shutdown() override;
    ~RendererGLES2() override;

    // Upload framebuffer data (RGBA8888) to the texture
    void upload_framebuffer(const uint8_t* pixels, int w, int h);

    // GXM command support
    void draw(uint32_t primType, uint32_t indexType, const void* indexData, uint32_t indexCount);
    void bindTexture(uint32_t textureUnit, uint32_t textureId);
    void createShader(const std::string& shaderName = "");
    void clear(uint32_t mask);
    void setState(uint32_t stateId, uint32_t value);
    // Texture and shader management
    void uploadTexture(uint32_t textureId, const void* data, int w, int h, int format);
    void useTexture(uint32_t textureUnit, uint32_t textureId);
    void createShaderFromSource(const std::string& name, const std::string& vertSrc, const std::string& fragSrc);
    void useShader(const std::string& name);
    // Vertex/index buffer upload
    void uploadVertexBuffer(const void* data, size_t size);
    void uploadIndexBuffer(const void* data, size_t size, uint32_t indexType);
    // Shader selection
    void setCurrentShader(const std::string& name);
    void setCurrentShaderById(uint32_t id);
    void setupVertexAttributes(const std::vector<std::pair<int, int>>& attribs);
    // Dynamic attribute setup
    void setupVertexAttributesDynamic(const std::vector<std::tuple<int, int, GLenum, size_t>>& layout, size_t stride);
    // Uniform and sampler management
    void setUniform(const std::string& name, float value);
    void setUniformVec(const std::string& name, const float* values, int count);
    void setSampler(const std::string& name, int unit);
    // Resource cleanup
    void cleanupResources();

private:
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    GLuint program = 0;
    GLuint vbo = 0;
    GLuint vao = 0;
    GLuint texture = 0;
    int width = 1280;
    int height = 720;
    int tex_width = 0;
    int tex_height = 0;
    bool create_context();
    bool create_shader();
    void destroy_context();
    void destroy_shader();
    void create_texture(int w, int h);
    // Texture and shader management
    std::unordered_map<uint32_t, GLuint> textures;
    std::unordered_map<std::string, GLuint> shaders;
    GLuint currentVBO = 0;
    GLuint currentIBO = 0;
    uint32_t currentIndexType = 0;
    std::string currentShaderName;
    uint32_t currentShaderId = 0;
}; 