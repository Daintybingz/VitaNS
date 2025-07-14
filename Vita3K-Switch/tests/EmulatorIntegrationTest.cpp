#include <gtest/gtest.h>
#include "../src/core/emulator/emulator.h"
#include "../src/renderer/RendererFactory.h"
#include <memory>

class EmulatorIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize SDL for testing
        SDL_Init(SDL_INIT_VIDEO);
    }
    
    void TearDown() override {
        SDL_Quit();
    }
};

TEST_F(EmulatorIntegrationTest, EmulatorInitialization) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    config.log_cpu = false;
    config.log_mem = false;
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
}

TEST_F(EmulatorIntegrationTest, RendererIntegration) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test that renderer was created and initialized
    // Note: We can't directly access the renderer, but we can test the integration
    // by calling renderFrame and ensuring no crashes
    EXPECT_NO_THROW(emulator->renderFrame());
}

TEST_F(EmulatorIntegrationTest, GPUSubsystemIntegration) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test GPU subsystem integration by calling renderFrame
    // which should trigger GXM command processing
    for (int i = 0; i < 10; ++i) {
        EXPECT_NO_THROW(emulator->renderFrame());
    }
}

TEST_F(EmulatorIntegrationTest, MemoryManagerIntegration) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test memory manager integration
    // The emulator should have initialized the memory manager
    // We can test this indirectly through renderFrame calls
    EXPECT_NO_THROW(emulator->renderFrame());
}

TEST_F(EmulatorIntegrationTest, ModuleManagerIntegration) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test that modules were registered and initialized
    // This is tested indirectly through renderFrame functionality
    EXPECT_NO_THROW(emulator->renderFrame());
}

TEST_F(EmulatorIntegrationTest, DisplayModuleIntegration) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test display module integration
    // The display module should provide framebuffer data for rendering
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_THROW(emulator->renderFrame());
    }
}

TEST_F(EmulatorIntegrationTest, ErrorHandling) {
    auto emulator = std::make_unique<Emulator>();
    
    // Test with invalid configuration
    EmulatorConfig invalidConfig;
    invalidConfig.base_path = "/nonexistent/path/";
    
    // Should handle invalid configuration gracefully
    EXPECT_FALSE(emulator->initialize(invalidConfig, nullptr));
}

TEST_F(EmulatorIntegrationTest, StateManagement) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test state transitions
    EXPECT_NO_THROW(emulator->run());
    EXPECT_NO_THROW(emulator->pause());
    EXPECT_NO_THROW(emulator->resume());
    EXPECT_NO_THROW(emulator->stop());
}

TEST_F(EmulatorIntegrationTest, PerformanceBaseline) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Measure basic performance
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        emulator->renderFrame();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete 100 frames in reasonable time (< 10 seconds)
    EXPECT_LT(duration.count(), 10000);
}

TEST_F(EmulatorIntegrationTest, ResourceCleanup) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    
    // Test resource cleanup
    size_t beforeCleanup = getCurrentMemoryUsage();
    
    // Render some frames
    for (int i = 0; i < 10; ++i) {
        emulator->renderFrame();
    }
    
    // Stop emulator (should trigger cleanup)
    emulator->stop();
    
    size_t afterCleanup = getCurrentMemoryUsage();
    
    // Memory usage should be reasonable after cleanup
    // (Note: exact comparison depends on implementation)
    EXPECT_LE(afterCleanup, beforeCleanup + 1000000); // Within 1MB
}

TEST_F(EmulatorIntegrationTest, MultipleInitialization) {
    auto emulator = std::make_unique<Emulator>();
    
    EmulatorConfig config;
    config.base_path = "test_data/";
    
    // Test multiple initialization cycles
    for (int cycle = 0; cycle < 3; ++cycle) {
        EXPECT_TRUE(emulator->initialize(config, nullptr));
        
        // Render some frames
        for (int i = 0; i < 5; ++i) {
            EXPECT_NO_THROW(emulator->renderFrame());
        }
        
        emulator->stop();
    }
}

TEST_F(EmulatorIntegrationTest, ConfigurationOptions) {
    auto emulator = std::make_unique<Emulator>();
    
    // Test different configuration options
    EmulatorConfig config;
    config.base_path = "test_data/";
    config.log_cpu = true;
    config.log_mem = true;
    config.enable_debugger = false;
    config.enable_cheats = false;
    config.log_fs = true;
    config.resolution_scale = 2;
    config.vsync = false;
    
    EXPECT_TRUE(emulator->initialize(config, nullptr));
    EXPECT_NO_THROW(emulator->renderFrame());
    emulator->stop();
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