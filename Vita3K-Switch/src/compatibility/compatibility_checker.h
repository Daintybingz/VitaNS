#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../shader/game_profiles.h"

namespace compatibility {

enum class CompatibilityStatus {
    Perfect,        // Game runs perfectly
    Playable,       // Game is playable with minor issues
    InGame,         // Game runs but has significant issues
    Menu,           // Game boots to menu only
    Intro,          // Game shows intro/title screen only
    Boots,          // Game boots but crashes
    Nothing,        // Game doesn't boot
    Unknown         // Not tested
};

enum class IssueType {
    Graphics,       // Graphics/rendering issues
    Performance,    // Performance/speed issues
    Audio,          // Audio issues
    Input,          // Input/control issues
    Saves,          // Save state issues
    Crashes,        // Stability/crash issues
    Compatibility,  // General compatibility issues
    Other           // Other issues
};

struct IssueReport {
    IssueType type;
    std::string description;
    std::string workaround;
    bool has_workaround;
    bool is_critical;
    std::string affected_hw_config;
    std::string affected_versions;
};

struct GameTestResult {
    std::string game_id;
    std::string game_name;
    std::string game_version;
    CompatibilityStatus status;
    std::vector<IssueReport> issues;
    float average_fps;
    float min_fps;
    float max_fps;
    uint32_t crashes_count;
    std::string tester;
    std::string test_date;
    std::string test_duration;
    std::string test_config;
    std::string notes;
};

class CompatibilityChecker {
public:
    CompatibilityChecker();
    ~CompatibilityChecker();

    // Database management
    bool initialize(const std::string& database_path);
    void save_database();
    void add_test_result(const GameTestResult& result);
    void update_test_result(const GameTestResult& result);
    GameTestResult* get_test_result(const std::string& game_id);
    std::vector<GameTestResult> get_all_results();

    // Testing
    void start_test_session(const std::string& game_id,
                           const std::string& tester);
    void end_test_session();
    void record_issue(const IssueReport& issue);
    void update_performance_metrics(float fps, float frame_time);
    void record_crash();

    // Analysis
    CompatibilityStatus analyze_compatibility();
    std::vector<IssueReport> analyze_issues();
    void generate_report(std::string& report);
    void suggest_fixes(std::vector<std::string>& suggestions);

    // Statistics
    struct CompatibilityStats {
        uint32_t total_games;
        uint32_t perfect_count;
        uint32_t playable_count;
        uint32_t ingame_count;
        uint32_t menu_count;
        uint32_t intro_count;
        uint32_t boots_count;
        uint32_t nothing_count;
        uint32_t unknown_count;
        float overall_compatibility;
    };

    CompatibilityStats get_stats() const;
    std::vector<std::string> get_most_compatible_games(uint32_t count = 10);
    std::vector<std::string> get_most_problematic_games(uint32_t count = 10);

    // Integration with game profiles
    void link_game_profile(shader::GameProfile* profile);
    void update_game_profile(const GameTestResult& result);
    void suggest_profile_optimizations(std::vector<std::string>& suggestions);

private:
    struct CompatibilityCheckerImpl;
    std::unique_ptr<CompatibilityCheckerImpl> impl;

    // Internal helpers
    void analyze_performance_issues(std::vector<IssueReport>& issues);
    void analyze_graphics_issues(std::vector<IssueReport>& issues);
    void analyze_audio_issues(std::vector<IssueReport>& issues);
    void analyze_input_issues(std::vector<IssueReport>& issues);
    void analyze_save_issues(std::vector<IssueReport>& issues);
    void analyze_crash_patterns(std::vector<IssueReport>& issues);

    // Database helpers
    void load_test_results();
    void save_test_results();
    void validate_test_result(GameTestResult& result);
    void merge_test_results(GameTestResult& target,
                          const GameTestResult& source);

    // Analysis helpers
    bool is_performance_acceptable(const GameTestResult& result);
    bool are_graphics_acceptable(const GameTestResult& result);
    bool is_stability_acceptable(const GameTestResult& result);
    void categorize_issues(const std::vector<IssueReport>& issues,
                         std::unordered_map<IssueType, uint32_t>& counts);

    // Reporting helpers
    void generate_performance_report(std::string& report);
    void generate_issues_report(std::string& report);
    void generate_compatibility_report(std::string& report);
    void generate_suggestions_report(std::string& report);
};

} // namespace compatibility 