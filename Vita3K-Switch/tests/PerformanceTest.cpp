#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include "../src/renderer/RendererFactory.h"
#include "../src/core/emulator/emulator.h"

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        SDL_Init(SDL_INIT_VIDEO);
    }
    
    void TearDown() override {
        SDL_Quit();
    }
    
    // Helper function to measure time
    template<typename Func>
    auto measureTime(Func func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }
    
    // Helper function to measure memory usage
    size_t getCurrentMemoryUsage() {
        // Platform-specific implementation would go here
        return 0; // Placeholder
    }
};

TEST_F(PerformanceTest, RendererInitializationPerformance) {
    std::cout << "\n=== Renderer Initialization Performance ===" << std::endl;
    
    // Test GLES2 renderer initialization
    auto gles2Time = measureTime([]() {
        auto renderer = RendererFactory::createRenderer(RendererType::GLES2);
        if (renderer) {
            renderer->init();
            renderer->shutdown();
        }
    });
    
    // Test Software renderer initialization
    auto softwareTime = measureTime([]() {
        auto renderer = RendererFactory::createRenderer(RendererType::Software);
        if (renderer) {
            renderer->init();
            renderer->shutdown();
        }
    });
    
    // Test Auto selection
    auto autoTime = measureTime([]() {
        auto renderer = RendererFactory::createBestRenderer();
        if (renderer) {
            renderer->init();
            renderer->shutdown();
        }
    });
    
    std::cout << "GLES2 Initialization: " << gles2Time.count() << " μs" << std::endl;
    std::cout << "Software Initialization: " << softwareTime.count() << " μs" << std::endl;
    std::cout << "Auto Selection: " << autoTime.count() << " μs" << std::endl;
    
    // Performance expectations
    EXPECT_LT(gles2Time.count(), 100000);    // < 100ms for GLES2
    EXPECT_LT(softwareTime.count(), 50000);  // < 50ms for Software
    EXPECT_LT(autoTime.count(), 150000);     // < 150ms for Auto
}

TEST_F(PerformanceTest, FramebufferUploadPerformance) {
    std::cout << "\n=== Framebuffer Upload Performance ===" << std::endl;
    
    // Test different resolutions
    std::vector<std::pair<int, int>> resolutions = {
        {640, 480},    // VGA
        {1280, 720},   // HD
        {1920, 1080},  // Full HD
        {2560, 1440},  // 2K
        {3840, 2160}   // 4K
    };
    
    for (const auto& res : resolutions) {
        int width = res.first;
        int height = res.second;
        size_t pixelCount = width * height * 4; // RGBA8888
        
        std::vector<uint8_t> pixels(pixelCount, 0xFF);
        
        // Test GLES2 upload
        auto gles2Time = measureTime([&]() {
            auto renderer = RendererFactory::createRenderer(RendererType::GLES2);
            if (renderer) {
                renderer->init();
                auto* gles2 = dynamic_cast<RendererGLES2*>(renderer.get());
                if (gles2) {
                    gles2->upload_framebuffer(pixels.data(), width, height);
                }
                renderer->shutdown();
            }
        });
        
        // Test Software upload
        auto softwareTime = measureTime([&]() {
            auto renderer = RendererFactory::createRenderer(RendererType::Software);
            if (renderer) {
                renderer->init();
                auto* software = dynamic_cast<RendererSoftware*>(renderer.get());
                if (software) {
                    software->upload_framebuffer(pixels.data(), width, height);
                }
                renderer->shutdown();
            }
        });
        
        std::cout << width << "x" << height << ":" << std::endl;
        std::cout << "  GLES2: " << gles2Time.count() << " μs" << std::endl;
        std::cout << "  Software: " << softwareTime.count() << " μs" << std::endl;
        
        // Performance expectations (scaled by resolution)
        size_t expectedGLES2 = pixelCount / 1000;  // Rough estimate
        size_t expectedSoftware = pixelCount / 500; // Rough estimate
        
        EXPECT_LT(gles2Time.count(), expectedGLES2);
        EXPECT_LT(softwareTime.count(), expectedSoftware);
    }
}

TEST_F(PerformanceTest, FrameRenderingPerformance) {
    std::cout << "\n=== Frame Rendering Performance ===" << std::endl;
    
    const int frameCount = 1000;
    
    // Test GLES2 rendering
    auto gles2Time = measureTime([&]() {
        auto renderer = RendererFactory::createRenderer(RendererType::GLES2);
        if (renderer) {
            renderer->init();
            
            std::vector<uint8_t> pixels(1280 * 720 * 4, 0xFF);
            auto* gles2 = dynamic_cast<RendererGLES2*>(renderer.get());
            if (gles2) {
                gles2->upload_framebuffer(pixels.data(), 1280, 720);
            }
            
            for (int i = 0; i < frameCount; ++i) {
                renderer->draw_frame();
                renderer->present();
            }
            
            renderer->shutdown();
        }
    });
    
    // Test Software rendering
    auto softwareTime = measureTime([&]() {
        auto renderer = RendererFactory::createRenderer(RendererType::Software);
        if (renderer) {
            renderer->init();
            
            std::vector<uint8_t> pixels(1280 * 720 * 4, 0xFF);
            auto* software = dynamic_cast<RendererSoftware*>(renderer.get());
            if (software) {
                software->upload_framebuffer(pixels.data(), 1280, 720);
            }
            
            for (int i = 0; i < frameCount; ++i) {
                renderer->draw_frame();
                renderer->present();
            }
            
            renderer->shutdown();
        }
    });
    
    double gles2FPS = (frameCount * 1000000.0) / gles2Time.count();
    double softwareFPS = (frameCount * 1000000.0) / softwareTime.count();
    
    std::cout << "GLES2: " << gles2FPS << " FPS (" << gles2Time.count() << " μs total)" << std::endl;
    std::cout << "Software: " << softwareFPS << " FPS (" << softwareTime.count() << " μs total)" << std::endl;
    
    // Performance expectations
    EXPECT_GT(gles2FPS, 30.0);     // At least 30 FPS for GLES2
    EXPECT_GT(softwareFPS, 15.0);  // At least 15 FPS for Software
}

TEST_F(PerformanceTest, EmulatorPerformance) {
    std::cout << "\n=== Emulator Performance ===" << std::endl;
    
    auto emulator = std::make_unique<Emulator>();
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    const int frameCount = 100;
    
    // Measure emulator frame rendering performance
    auto emulatorTime = measureTime([&]() {
        for (int i = 0; i < frameCount; ++i) {
            emulator->renderFrame();
        }
    });
    
    double emulatorFPS = (frameCount * 1000000.0) / emulatorTime.count();
    
    std::cout << "Emulator: " << emulatorFPS << " FPS (" << emulatorTime.count() << " μs total)" << std::endl;
    
    // Performance expectations
    EXPECT_GT(emulatorFPS, 20.0);  // At least 20 FPS for emulator
    
    emulator->stop();
}

TEST_F(PerformanceTest, MemoryUsagePerformance) {
    std::cout << "\n=== Memory Usage Performance ===" << std::endl;
    
    size_t initialMemory = getCurrentMemoryUsage();
    
    // Test GLES2 memory usage
    auto renderer = RendererFactory::createRenderer(RendererType::GLES2);
    if (renderer) {
        renderer->init();
        
        size_t afterInit = getCurrentMemoryUsage();
        size_t initMemory = afterInit - initialMemory;
        
        // Upload different sized textures
        for (int size = 256; size <= 2048; size *= 2) {
            std::vector<uint8_t> pixels(size * size * 4, 0xFF);
            auto* gles2 = dynamic_cast<RendererGLES2*>(renderer.get());
            if (gles2) {
                gles2->upload_framebuffer(pixels.data(), size, size);
            }
            
            size_t afterUpload = getCurrentMemoryUsage();
            size_t uploadMemory = afterUpload - afterInit;
            
            std::cout << size << "x" << size << " texture:" << std::endl;
            std::cout << "  Init memory: " << initMemory << " bytes" << std::endl;
            std::cout << "  Upload memory: " << uploadMemory << " bytes" << std::endl;
            
            // Memory expectations
            EXPECT_LT(initMemory, 100 * 1024 * 1024);  // < 100MB for init
            EXPECT_LT(uploadMemory, size * size * 8);  // < 8x texture size for upload
        }
        
        renderer->shutdown();
    }
    
    size_t finalMemory = getCurrentMemoryUsage();
    size_t cleanupMemory = finalMemory - initialMemory;
    
    std::cout << "Final memory delta: " << cleanupMemory << " bytes" << std::endl;
    
    // Should clean up most memory
    EXPECT_LT(cleanupMemory, 10 * 1024 * 1024);  // < 10MB remaining
}

TEST_F(PerformanceTest, StressTest) {
    std::cout << "\n=== Stress Test ===" << std::endl;
    
    const int cycles = 10;
    const int framesPerCycle = 100;
    
    auto totalTime = measureTime([&]() {
        for (int cycle = 0; cycle < cycles; ++cycle) {
            auto renderer = RendererFactory::createBestRenderer();
            if (renderer) {
                renderer->init();
                
                std::vector<uint8_t> pixels(1280 * 720 * 4, 0xFF);
                auto* gles2 = dynamic_cast<RendererGLES2*>(renderer.get());
                auto* software = dynamic_cast<RendererSoftware*>(renderer.get());
                
                if (gles2) {
                    gles2->upload_framebuffer(pixels.data(), 1280, 720);
                } else if (software) {
                    software->upload_framebuffer(pixels.data(), 1280, 720);
                }
                
                for (int frame = 0; frame < framesPerCycle; ++frame) {
                    renderer->draw_frame();
                    renderer->present();
                }
                
                renderer->shutdown();
            }
        }
    });
    
    double totalFPS = (cycles * framesPerCycle * 1000000.0) / totalTime.count();
    
    std::cout << "Stress test: " << totalFPS << " FPS (" << totalTime.count() << " μs total)" << std::endl;
    std::cout << "Cycles: " << cycles << ", Frames per cycle: " << framesPerCycle << std::endl;
    
    // Stress test expectations
    EXPECT_GT(totalFPS, 15.0);  // At least 15 FPS under stress
    EXPECT_LT(totalTime.count(), 10000000);  // < 10 seconds total
}

TEST_F(PerformanceTest, PlatformComparison) {
    std::cout << "\n=== Platform Comparison ===" << std::endl;
    
    // Test different renderer types and compare performance
    std::vector<RendererType> rendererTypes = {
        RendererType::GLES2,
        RendererType::Software
    };
    
    for (auto type : rendererTypes) {
        auto renderer = RendererFactory::createRenderer(type);
        if (renderer) {
            auto initTime = measureTime([&]() {
                renderer->init();
            });
            
            std::vector<uint8_t> pixels(1280 * 720 * 4, 0xFF);
            
            auto uploadTime = measureTime([&]() {
                auto* gles2 = dynamic_cast<RendererGLES2*>(renderer.get());
                auto* software = dynamic_cast<RendererSoftware*>(renderer.get());
                
                if (gles2) {
                    gles2->upload_framebuffer(pixels.data(), 1280, 720);
                } else if (software) {
                    software->upload_framebuffer(pixels.data(), 1280, 720);
                }
            });
            
            const int frameCount = 100;
            auto renderTime = measureTime([&]() {
                for (int i = 0; i < frameCount; ++i) {
                    renderer->draw_frame();
                    renderer->present();
                }
            });
            
            double fps = (frameCount * 1000000.0) / renderTime.count();
            
            std::string typeName = (type == RendererType::GLES2) ? "GLES2" : "Software";
            std::cout << typeName << ":" << std::endl;
            std::cout << "  Init: " << initTime.count() << " μs" << std::endl;
            std::cout << "  Upload: " << uploadTime.count() << " μs" << std::endl;
            std::cout << "  Render: " << fps << " FPS" << std::endl;
            
            renderer->shutdown();
        }
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Set up performance test output
    testing::GTEST_FLAG(output) = "xml:performance_test_results.xml";
    
    return RUN_ALL_TESTS();
} 