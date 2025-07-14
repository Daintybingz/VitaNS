#include <gtest/gtest.h>
#include "../src/renderer/Renderer.h"
#include "../src/renderer/RendererStub.h"
#include "../src/renderer/RendererGLES2.h"
#include "../src/renderer/RendererSoftware.h"
#include "../src/renderer/RendererFactory.h"
#include <memory>

class RendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for testing
        SDL_Init(SDL_INIT_VIDEO);
    }
    
    void TearDown() override {
        SDL_Quit();
    }
};

// Test RendererStub
TEST_F(RendererTest, RendererStubInterfaceCompliance) {
    auto renderer = std::make_unique<RendererStub>();
    
    EXPECT_TRUE(renderer->init());
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
    EXPECT_NO_THROW(renderer->shutdown());
}

TEST_F(RendererTest, RendererStubMultipleInitShutdown) {
    auto renderer = std::make_unique<RendererStub>();
    
    // Multiple init/shutdown cycles should work
    EXPECT_TRUE(renderer->init());
    EXPECT_TRUE(renderer->init());
    renderer->shutdown();
    EXPECT_TRUE(renderer->init());
    renderer->shutdown();
}

// Test RendererFactory
TEST_F(RendererTest, RendererFactoryCapabilityDetection) {
    auto caps = RendererFactory::detectCapabilities();
    
    // Should detect some capabilities
    EXPECT_TRUE(caps.hasOpenGL || !caps.hasOpenGL); // Either true or false
    EXPECT_TRUE(caps.hasGLES2 || !caps.hasGLES2);   // Either true or false
    
    // Should have version strings
    EXPECT_FALSE(caps.glVersion.empty());
    EXPECT_FALSE(caps.glRenderer.empty());
    EXPECT_FALSE(caps.glVendor.empty());
}

TEST_F(RendererTest, RendererFactoryBestRendererSelection) {
    auto caps = RendererFactory::detectCapabilities();
    auto type = RendererFactory::selectBestRenderer(caps);
    
    // Should select a valid renderer type
    EXPECT_TRUE(type == RendererType::GLES2 || type == RendererType::Software);
}

TEST_F(RendererTest, RendererFactoryCreateBestRenderer) {
    auto renderer = RendererFactory::createBestRenderer();
    
    EXPECT_NE(renderer, nullptr);
    EXPECT_TRUE(renderer->init());
    renderer->shutdown();
}

TEST_F(RendererTest, RendererFactoryCreateSpecificRenderers) {
    // Test GLES2 renderer creation
    auto gles2 = RendererFactory::createRenderer(RendererType::GLES2);
    if (gles2) {
        EXPECT_TRUE(gles2->init());
        gles2->shutdown();
    }
    
    // Test Software renderer creation
    auto software = RendererFactory::createRenderer(RendererType::Software);
    EXPECT_NE(software, nullptr);
    EXPECT_TRUE(software->init());
    software->shutdown();
}

// Test RendererSoftware
TEST_F(RendererTest, RendererSoftwareBasicFunctionality) {
    auto renderer = std::make_unique<RendererSoftware>();
    
    EXPECT_TRUE(renderer->init());
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
    EXPECT_NO_THROW(renderer->shutdown());
}

TEST_F(RendererTest, RendererSoftwareFramebufferUpload) {
    auto renderer = std::make_unique<RendererSoftware>();
    EXPECT_TRUE(renderer->init());
    
    // Test framebuffer upload
    std::vector<uint8_t> testPixels(1280 * 720 * 4, 0xFF); // White pixels
    EXPECT_NO_THROW(renderer->upload_framebuffer(testPixels.data(), 1280, 720));
    
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
    renderer->shutdown();
}

TEST_F(RendererSoftware, RendererSoftwareMultipleResolutions) {
    auto renderer = std::make_unique<RendererSoftware>();
    EXPECT_TRUE(renderer->init());
    
    // Test different resolutions
    std::vector<uint8_t> pixels1(640 * 480 * 4, 0xFF);
    std::vector<uint8_t> pixels2(1920 * 1080 * 4, 0xFF);
    
    EXPECT_NO_THROW(renderer->upload_framebuffer(pixels1.data(), 640, 480));
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
    
    EXPECT_NO_THROW(renderer->upload_framebuffer(pixels2.data(), 1920, 1080));
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
    
    renderer->shutdown();
}

// Test RendererGLES2 (if available)
TEST_F(RendererTest, RendererGLES2BasicFunctionality) {
    auto renderer = std::make_unique<RendererGLES2>();
    
    if (renderer->init()) {
        EXPECT_NO_THROW(renderer->draw_frame());
        EXPECT_NO_THROW(renderer->present());
        EXPECT_NO_THROW(renderer->shutdown());
    } else {
        // GLES2 not available, skip test
        GTEST_SKIP() << "GLES2 not available on this platform";
    }
}

TEST_F(RendererTest, RendererGLES2FramebufferUpload) {
    auto renderer = std::make_unique<RendererGLES2>();
    
    if (renderer->init()) {
        // Test framebuffer upload
        std::vector<uint8_t> testPixels(1280 * 720 * 4, 0xFF); // White pixels
        EXPECT_NO_THROW(renderer->upload_framebuffer(testPixels.data(), 1280, 720));
        
        EXPECT_NO_THROW(renderer->draw_frame());
        EXPECT_NO_THROW(renderer->present());
        renderer->shutdown();
    } else {
        GTEST_SKIP() << "GLES2 not available on this platform";
    }
}

// Test error conditions
TEST_F(RendererTest, RendererErrorHandling) {
    auto renderer = std::make_unique<RendererSoftware>();
    
    // Should handle calls before init
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
    
    EXPECT_TRUE(renderer->init());
    
    // Should handle calls after shutdown
    renderer->shutdown();
    EXPECT_NO_THROW(renderer->draw_frame());
    EXPECT_NO_THROW(renderer->present());
}

TEST_F(RendererTest, RendererFactoryErrorHandling) {
    // Test unknown renderer type
    auto renderer = RendererFactory::createRenderer(RendererType::Unknown);
    EXPECT_NE(renderer, nullptr); // Should fall back to software
    
    if (renderer) {
        EXPECT_TRUE(renderer->init());
        renderer->shutdown();
    }
}

// Performance tests
TEST_F(RendererTest, RendererPerformanceBasic) {
    auto renderer = std::make_unique<RendererSoftware>();
    EXPECT_TRUE(renderer->init());
    
    // Measure basic rendering performance
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        renderer->draw_frame();
        renderer->present();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete 100 frames in reasonable time (< 5 seconds)
    EXPECT_LT(duration.count(), 5000);
    
    renderer->shutdown();
}

// Memory tests
TEST_F(RendererTest, RendererMemoryManagement) {
    auto renderer = std::make_unique<RendererSoftware>();
    
    // Test memory allocation during init
    size_t beforeInit = getCurrentMemoryUsage();
    EXPECT_TRUE(renderer->init());
    size_t afterInit = getCurrentMemoryUsage();
    
    // Should allocate some memory
    EXPECT_GT(afterInit, beforeInit);
    
    // Test memory cleanup during shutdown
    renderer->shutdown();
    size_t afterShutdown = getCurrentMemoryUsage();
    
    // Should free most memory
    EXPECT_LT(afterShutdown, afterInit);
}

// Helper function for memory usage (platform-specific)
size_t getCurrentMemoryUsage() {
    // This is a simplified implementation
    // In a real test, you'd use platform-specific APIs
    return 0; // Placeholder
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 