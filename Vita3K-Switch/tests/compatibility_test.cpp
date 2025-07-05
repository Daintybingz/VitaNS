#include <gtest/gtest.h>
#include "../src/compatibility/compatibility_checker.h"
#include <filesystem>
#include <fstream>

using namespace compatibility;

class CompatibilityCheckerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary database file
        temp_db_path = std::filesystem::temp_directory_path() / "test_compatibility.json";
        checker.initialize(temp_db_path.string());
    }

    void TearDown() override {
        // Clean up temporary file
        std::filesystem::remove(temp_db_path);
    }

    CompatibilityChecker checker;
    std::filesystem::path temp_db_path;
};

TEST_F(CompatibilityCheckerTest, BasicTestSession) {
    // Start a test session
    checker.start_test_session("TEST001", "test_user");

    // Record some performance metrics
    for (int i = 0; i < 100; i++) {
        checker.update_performance_metrics(30.0f + (rand() % 10), 33.33f);
    }

    // Record some issues
    IssueReport graphics_issue{
        .type = IssueType::Graphics,
        .description = "Texture corruption in menu",
        .workaround = "Update GPU drivers",
        .has_workaround = true,
        .is_critical = false,
        .affected_hw_config = "All",
        .affected_versions = "1.0.0"
    };
    checker.record_issue(graphics_issue);

    // End session and verify results
    checker.end_test_session();

    GameTestResult* result = checker.get_test_result("TEST001");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->game_id, "TEST001");
    EXPECT_EQ(result->issues.size(), 1);
    EXPECT_GE(result->average_fps, 30.0f);
}

TEST_F(CompatibilityCheckerTest, CrashHandling) {
    checker.start_test_session("TEST002", "test_user");

    // Simulate some crashes
    for (int i = 0; i < 5; i++) {
        checker.record_crash();
    }

    checker.end_test_session();

    GameTestResult* result = checker.get_test_result("TEST002");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->crashes_count, 5);
    EXPECT_EQ(result->status, CompatibilityStatus::Boots);
}

TEST_F(CompatibilityCheckerTest, PerfectCompatibility) {
    checker.start_test_session("TEST003", "test_user");

    // Simulate perfect performance
    for (int i = 0; i < 100; i++) {
        checker.update_performance_metrics(60.0f, 16.67f);
    }

    checker.end_test_session();

    GameTestResult* result = checker.get_test_result("TEST003");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->status, CompatibilityStatus::Perfect);
}

TEST_F(CompatibilityCheckerTest, StatisticsGeneration) {
    // Add multiple test results
    checker.start_test_session("TEST004", "test_user");
    for (int i = 0; i < 100; i++) {
        checker.update_performance_metrics(30.0f, 33.33f);
    }
    checker.end_test_session();

    checker.start_test_session("TEST005", "test_user");
    for (int i = 0; i < 100; i++) {
        checker.update_performance_metrics(60.0f, 16.67f);
    }
    checker.end_test_session();

    checker.start_test_session("TEST006", "test_user");
    checker.record_crash();
    checker.end_test_session();

    // Verify statistics
    CompatibilityChecker::CompatibilityStats stats = checker.get_stats();
    EXPECT_EQ(stats.total_games, 3);
    EXPECT_GT(stats.overall_compatibility, 0.0f);
}

TEST_F(CompatibilityCheckerTest, GameProfileIntegration) {
    shader::GameProfile profile;
    checker.link_game_profile(&profile);

    checker.start_test_session("TEST007", "test_user");

    // Simulate heavy compute usage
    for (int i = 0; i < 100; i++) {
        checker.update_performance_metrics(25.0f, 40.0f);
    }

    IssueReport performance_issue{
        .type = IssueType::Performance,
        .description = "Heavy compute shader usage",
        .has_workaround = false,
        .is_critical = true
    };
    checker.record_issue(performance_issue);

    checker.end_test_session();

    // Verify profile updates
    GameTestResult* result = checker.get_test_result("TEST007");
    ASSERT_NE(result, nullptr);
    checker.update_game_profile(*result);

    EXPECT_TRUE(profile.quirks.heavy_compute_usage);
    EXPECT_EQ(profile.target_fps, 25.0f);
}

TEST_F(CompatibilityCheckerTest, ReportGeneration) {
    checker.start_test_session("TEST008", "test_user");

    // Add various types of data
    for (int i = 0; i < 100; i++) {
        checker.update_performance_metrics(30.0f + (rand() % 10), 33.33f);
    }

    checker.record_issue(IssueReport{
        .type = IssueType::Graphics,
        .description = "Texture issues",
        .has_workaround = true,
        .is_critical = false
    });

    checker.record_crash();

    checker.end_test_session();

    // Generate and verify reports
    std::string report;
    checker.generate_report(report);
    EXPECT_FALSE(report.empty());

    std::vector<std::string> suggestions;
    checker.suggest_fixes(suggestions);
    EXPECT_FALSE(suggestions.empty());
}

TEST_F(CompatibilityCheckerTest, DatabasePersistence) {
    // Add some test results
    checker.start_test_session("TEST009", "test_user");
    checker.end_test_session();

    // Save database
    EXPECT_TRUE(checker.save_database());

    // Create new checker instance and load database
    CompatibilityChecker new_checker;
    new_checker.initialize(temp_db_path.string());

    // Verify data persistence
    GameTestResult* result = new_checker.get_test_result("TEST009");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->game_id, "TEST009");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 