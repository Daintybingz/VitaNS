#include "compatibility_checker.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <mutex>
#include <json.hpp>

namespace compatibility {

using json = nlohmann::json;

struct CompatibilityChecker::CompatibilityCheckerImpl {
    std::string database_path;
    std::unordered_map<std::string, GameTestResult> test_results;
    
    // Current test session
    struct TestSession {
        std::string game_id;
        std::string tester;
        std::chrono::system_clock::time_point start_time;
        std::vector<float> fps_history;
        std::vector<float> frame_time_history;
        uint32_t crash_count;
        std::vector<IssueReport> issues;
    } current_session;
    
    // Game profile integration
    shader::GameProfile* linked_profile;
    
    // Constants
    static constexpr float MIN_ACCEPTABLE_FPS = 25.0f;
    static constexpr float TARGET_FPS = 30.0f;
    static constexpr float MAX_FRAME_TIME = 33.33f; // ms
    static constexpr uint32_t MIN_TEST_DURATION = 300; // seconds
    static constexpr uint32_t MAX_CRASHES = 3;
};

CompatibilityChecker::CompatibilityChecker() : impl(std::make_unique<CompatibilityCheckerImpl>()) {
    impl->linked_profile = nullptr;
}

CompatibilityChecker::~CompatibilityChecker() = default;

bool CompatibilityChecker::initialize(const std::string& database_path) {
    impl->database_path = database_path;
    load_test_results();
    return true;
}

void CompatibilityChecker::save_database() {
    save_test_results();
}

void CompatibilityChecker::add_test_result(const GameTestResult& result) {
    GameTestResult validated_result = result;
    validate_test_result(validated_result);
    
    auto it = impl->test_results.find(result.game_id);
    if (it != impl->test_results.end()) {
        merge_test_results(it->second, validated_result);
    } else {
        impl->test_results[result.game_id] = validated_result;
    }
    
    save_database();
}

void CompatibilityChecker::update_test_result(const GameTestResult& result) {
    validate_test_result(const_cast<GameTestResult&>(result));
    impl->test_results[result.game_id] = result;
    save_database();
}

GameTestResult* CompatibilityChecker::get_test_result(const std::string& game_id) {
    auto it = impl->test_results.find(game_id);
    return it != impl->test_results.end() ? &it->second : nullptr;
}

std::vector<GameTestResult> CompatibilityChecker::get_all_results() {
    std::vector<GameTestResult> results;
    results.reserve(impl->test_results.size());
    
    for (const auto& [_, result] : impl->test_results) {
        results.push_back(result);
    }
    
    return results;
}

void CompatibilityChecker::start_test_session(const std::string& game_id,
                                            const std::string& tester) {
    impl->current_session = CompatibilityCheckerImpl::TestSession{
        .game_id = game_id,
        .tester = tester,
        .start_time = std::chrono::system_clock::now(),
        .crash_count = 0
    };
}

void CompatibilityChecker::end_test_session() {
    auto& session = impl->current_session;
    if (session.game_id.empty()) return;
    
    // Calculate test duration
    auto end_time = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        end_time - session.start_time).count();
    
    // Calculate performance metrics
    float avg_fps = 0.0f;
    float min_fps = std::numeric_limits<float>::max();
    float max_fps = 0.0f;
    
    for (float fps : session.fps_history) {
        avg_fps += fps;
        min_fps = std::min(min_fps, fps);
        max_fps = std::max(max_fps, fps);
    }
    
    if (!session.fps_history.empty()) {
        avg_fps /= session.fps_history.size();
    }
    
    // Create test result
    GameTestResult result{
        .game_id = session.game_id,
        .game_name = "", // TODO: Get from game database
        .game_version = "", // TODO: Get from game
        .status = analyze_compatibility(),
        .issues = session.issues,
        .average_fps = avg_fps,
        .min_fps = min_fps,
        .max_fps = max_fps,
        .crashes_count = session.crash_count,
        .tester = session.tester,
        .test_date = std::to_string(std::chrono::system_clock::to_time_t(end_time)),
        .test_duration = std::to_string(duration),
        .test_config = "" // TODO: Get system config
    };
    
    add_test_result(result);
    
    // Clear session
    session = CompatibilityCheckerImpl::TestSession{};
}

void CompatibilityChecker::record_issue(const IssueReport& issue) {
    impl->current_session.issues.push_back(issue);
}

void CompatibilityChecker::update_performance_metrics(float fps, float frame_time) {
    impl->current_session.fps_history.push_back(fps);
    impl->current_session.frame_time_history.push_back(frame_time);
}

void CompatibilityChecker::record_crash() {
    impl->current_session.crash_count++;
}

CompatibilityStatus CompatibilityChecker::analyze_compatibility() {
    const auto& session = impl->current_session;
    
    // Check for critical issues
    bool has_critical_issues = false;
    for (const auto& issue : session.issues) {
        if (issue.is_critical) {
            has_critical_issues = true;
            break;
        }
    }
    
    // Analyze performance
    float avg_fps = 0.0f;
    for (float fps : session.fps_history) {
        avg_fps += fps;
    }
    avg_fps /= session.fps_history.size();
    
    // Determine status
    if (session.crash_count > CompatibilityCheckerImpl::MAX_CRASHES) {
        return CompatibilityStatus::Boots;
    }
    
    if (has_critical_issues) {
        return CompatibilityStatus::InGame;
    }
    
    if (avg_fps < CompatibilityCheckerImpl::MIN_ACCEPTABLE_FPS) {
        return CompatibilityStatus::InGame;
    }
    
    if (session.issues.empty() && avg_fps >= CompatibilityCheckerImpl::TARGET_FPS) {
        return CompatibilityStatus::Perfect;
    }
    
    return CompatibilityStatus::Playable;
}

std::vector<IssueReport> CompatibilityChecker::analyze_issues() {
    std::vector<IssueReport> all_issues;
    
    analyze_performance_issues(all_issues);
    analyze_graphics_issues(all_issues);
    analyze_audio_issues(all_issues);
    analyze_input_issues(all_issues);
    analyze_save_issues(all_issues);
    analyze_crash_patterns(all_issues);
    
    return all_issues;
}

void CompatibilityChecker::generate_report(std::string& report) {
    std::stringstream ss;
    
    // Generate report sections
    std::string compatibility_report;
    generate_compatibility_report(compatibility_report);
    ss << compatibility_report << "\n\n";
    
    std::string performance_report;
    generate_performance_report(performance_report);
    ss << performance_report << "\n\n";
    
    std::string issues_report;
    generate_issues_report(issues_report);
    ss << issues_report;
    
    report = ss.str();
}

void CompatibilityChecker::suggest_fixes(std::vector<std::string>& suggestions) {
    const auto& session = impl->current_session;
    
    // Analyze performance issues
    float avg_fps = 0.0f;
    for (float fps : session.fps_history) {
        avg_fps += fps;
    }
    avg_fps /= session.fps_history.size();
    
    if (avg_fps < CompatibilityCheckerImpl::TARGET_FPS) {
        suggestions.push_back("Enable performance mode in game profile");
        suggestions.push_back("Reduce resolution scale");
        suggestions.push_back("Disable non-essential post-processing effects");
    }
    
    // Analyze stability issues
    if (session.crash_count > 0) {
        suggestions.push_back("Update to latest GPU drivers");
        suggestions.push_back("Verify game files");
        suggestions.push_back("Check system requirements");
    }
    
    // Analyze graphics issues
    bool has_graphics_issues = false;
    for (const auto& issue : session.issues) {
        if (issue.type == IssueType::Graphics) {
            has_graphics_issues = true;
            break;
        }
    }
    
    if (has_graphics_issues) {
        suggestions.push_back("Update shader cache");
        suggestions.push_back("Try different graphics settings");
        suggestions.push_back("Check for known shader issues");
    }
}

CompatibilityChecker::CompatibilityStats CompatibilityChecker::get_stats() const {
    CompatibilityStats stats = {0};
    
    for (const auto& [_, result] : impl->test_results) {
        stats.total_games++;
        
        switch (result.status) {
            case CompatibilityStatus::Perfect:
                stats.perfect_count++;
                break;
            case CompatibilityStatus::Playable:
                stats.playable_count++;
                break;
            case CompatibilityStatus::InGame:
                stats.ingame_count++;
                break;
            case CompatibilityStatus::Menu:
                stats.menu_count++;
                break;
            case CompatibilityStatus::Intro:
                stats.intro_count++;
                break;
            case CompatibilityStatus::Boots:
                stats.boots_count++;
                break;
            case CompatibilityStatus::Nothing:
                stats.nothing_count++;
                break;
            case CompatibilityStatus::Unknown:
                stats.unknown_count++;
                break;
        }
    }
    
    if (stats.total_games > 0) {
        stats.overall_compatibility = static_cast<float>(
            stats.perfect_count + stats.playable_count) / stats.total_games;
    }
    
    return stats;
}

std::vector<std::string> CompatibilityChecker::get_most_compatible_games(uint32_t count) {
    std::vector<std::pair<std::string, CompatibilityStatus>> games;
    games.reserve(impl->test_results.size());
    
    for (const auto& [game_id, result] : impl->test_results) {
        games.emplace_back(game_id, result.status);
    }
    
    std::sort(games.begin(), games.end(),
        [](const auto& a, const auto& b) {
            return static_cast<int>(a.second) < static_cast<int>(b.second);
        });
    
    std::vector<std::string> result;
    result.reserve(std::min(count, static_cast<uint32_t>(games.size())));
    
    for (size_t i = 0; i < count && i < games.size(); i++) {
        result.push_back(games[i].first);
    }
    
    return result;
}

std::vector<std::string> CompatibilityChecker::get_most_problematic_games(uint32_t count) {
    std::vector<std::pair<std::string, uint32_t>> games;
    games.reserve(impl->test_results.size());
    
    for (const auto& [game_id, result] : impl->test_results) {
        games.emplace_back(game_id, result.issues.size());
    }
    
    std::sort(games.begin(), games.end(),
        [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
    
    std::vector<std::string> result;
    result.reserve(std::min(count, static_cast<uint32_t>(games.size())));
    
    for (size_t i = 0; i < count && i < games.size(); i++) {
        result.push_back(games[i].first);
    }
    
    return result;
}

void CompatibilityChecker::link_game_profile(shader::GameProfile* profile) {
    impl->linked_profile = profile;
}

void CompatibilityChecker::update_game_profile(const GameTestResult& result) {
    if (!impl->linked_profile) return;
    
    // Update performance targets
    impl->linked_profile->target_fps = result.average_fps;
    impl->linked_profile->min_fps = result.min_fps;
    
    // Update quirks based on issues
    for (const auto& issue : result.issues) {
        switch (issue.type) {
            case IssueType::Graphics:
                impl->linked_profile->quirks.needs_vertex_cache_flush = true;
                break;
            case IssueType::Performance:
                impl->linked_profile->quirks.heavy_compute_usage = true;
                break;
            default:
                break;
        }
    }
}

void CompatibilityChecker::suggest_profile_optimizations(std::vector<std::string>& suggestions) {
    if (!impl->linked_profile) return;
    
    const auto& profile = *impl->linked_profile;
    
    // Suggest shader optimizations
    if (profile.quirks.heavy_compute_usage) {
        suggestions.push_back("Enable compute shader optimizations");
        suggestions.push_back("Increase shared memory allocation");
    }
    
    // Suggest memory optimizations
    if (profile.quirks.texture_heavy) {
        suggestions.push_back("Enable texture compression");
        suggestions.push_back("Optimize texture cache size");
    }
    
    // Suggest performance optimizations
    if (profile.characteristics.avg_shader_complexity > 0.8f) {
        suggestions.push_back("Enable aggressive shader optimization");
        suggestions.push_back("Reduce post-processing effects");
    }
}

// Private methods

void CompatibilityChecker::load_test_results() {
    try {
        std::ifstream file(impl->database_path);
        if (!file.is_open()) return;
        
        json j;
        file >> j;
        
        for (const auto& result : j) {
            GameTestResult test_result;
            test_result.game_id = result["game_id"];
            test_result.game_name = result["game_name"];
            test_result.game_version = result["game_version"];
            test_result.status = static_cast<CompatibilityStatus>(result["status"]);
            test_result.average_fps = result["average_fps"];
            test_result.min_fps = result["min_fps"];
            test_result.max_fps = result["max_fps"];
            test_result.crashes_count = result["crashes_count"];
            test_result.tester = result["tester"];
            test_result.test_date = result["test_date"];
            test_result.test_duration = result["test_duration"];
            test_result.test_config = result["test_config"];
            test_result.notes = result["notes"];
            
            for (const auto& issue : result["issues"]) {
                IssueReport report;
                report.type = static_cast<IssueType>(issue["type"]);
                report.description = issue["description"];
                report.workaround = issue["workaround"];
                report.has_workaround = issue["has_workaround"];
                report.is_critical = issue["is_critical"];
                report.affected_hw_config = issue["affected_hw_config"];
                report.affected_versions = issue["affected_versions"];
                test_result.issues.push_back(report);
            }
            
            impl->test_results[test_result.game_id] = test_result;
        }
    } catch (const std::exception& e) {
        // Handle loading errors
    }
}

void CompatibilityChecker::save_test_results() {
    try {
        json j = json::array();
        
        for (const auto& [_, result] : impl->test_results) {
            json test_result;
            test_result["game_id"] = result.game_id;
            test_result["game_name"] = result.game_name;
            test_result["game_version"] = result.game_version;
            test_result["status"] = static_cast<int>(result.status);
            test_result["average_fps"] = result.average_fps;
            test_result["min_fps"] = result.min_fps;
            test_result["max_fps"] = result.max_fps;
            test_result["crashes_count"] = result.crashes_count;
            test_result["tester"] = result.tester;
            test_result["test_date"] = result.test_date;
            test_result["test_duration"] = result.test_duration;
            test_result["test_config"] = result.test_config;
            test_result["notes"] = result.notes;
            
            json issues = json::array();
            for (const auto& issue : result.issues) {
                json issue_report;
                issue_report["type"] = static_cast<int>(issue.type);
                issue_report["description"] = issue.description;
                issue_report["workaround"] = issue.workaround;
                issue_report["has_workaround"] = issue.has_workaround;
                issue_report["is_critical"] = issue.is_critical;
                issue_report["affected_hw_config"] = issue.affected_hw_config;
                issue_report["affected_versions"] = issue.affected_versions;
                issues.push_back(issue_report);
            }
            test_result["issues"] = issues;
            
            j.push_back(test_result);
        }
        
        std::ofstream file(impl->database_path);
        file << j.dump(4);
        return;
    } catch (const std::exception& e) {
        // Handle saving errors
    }
}

void CompatibilityChecker::validate_test_result(GameTestResult& result) {
    // Validate numeric values
    result.average_fps = std::max(0.0f, result.average_fps);
    result.min_fps = std::max(0.0f, result.min_fps);
    result.max_fps = std::max(0.0f, result.max_fps);
    result.crashes_count = std::max(0u, result.crashes_count);
    
    // Ensure required fields are not empty
    if (result.game_id.empty()) {
        result.game_id = "unknown";
    }
    if (result.test_date.empty()) {
        result.test_date = std::to_string(
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    }
    
    // Validate status
    if (result.status == CompatibilityStatus::Unknown) {
        result.status = analyze_compatibility();
    }
}

void CompatibilityChecker::merge_test_results(GameTestResult& target,
                                            const GameTestResult& source) {
    // Keep the most recent test date
    if (std::stoull(source.test_date) > std::stoull(target.test_date)) {
        target = source;
        return;
    }
    
    // Merge performance metrics
    target.average_fps = (target.average_fps + source.average_fps) / 2.0f;
    target.min_fps = std::min(target.min_fps, source.min_fps);
    target.max_fps = std::max(target.max_fps, source.max_fps);
    target.crashes_count += source.crashes_count;
    
    // Merge issues
    for (const auto& issue : source.issues) {
        bool found = false;
        for (const auto& existing : target.issues) {
            if (issue.type == existing.type &&
                issue.description == existing.description) {
                found = true;
                break;
            }
        }
        if (!found) {
            target.issues.push_back(issue);
        }
    }
    
    // Update status to the worse one
    if (static_cast<int>(source.status) > static_cast<int>(target.status)) {
        target.status = source.status;
    }
}

void CompatibilityChecker::analyze_performance_issues(std::vector<IssueReport>& issues) {
    const auto& session = impl->current_session;
    
    // Check average FPS
    float avg_fps = 0.0f;
    for (float fps : session.fps_history) {
        avg_fps += fps;
    }
    avg_fps /= session.fps_history.size();
    
    if (avg_fps < CompatibilityCheckerImpl::MIN_ACCEPTABLE_FPS) {
        issues.push_back(IssueReport{
            .type = IssueType::Performance,
            .description = "Low average FPS",
            .workaround = "Try reducing graphics settings or enabling performance mode",
            .has_workaround = true,
            .is_critical = true
        });
    }
    
    // Check frame time spikes
    for (float frame_time : session.frame_time_history) {
        if (frame_time > CompatibilityCheckerImpl::MAX_FRAME_TIME) {
            issues.push_back(IssueReport{
                .type = IssueType::Performance,
                .description = "Frame time spikes detected",
                .workaround = "Check for background processes or thermal throttling",
                .has_workaround = true,
                .is_critical = false
            });
            break;
        }
    }
}

void CompatibilityChecker::analyze_graphics_issues(std::vector<IssueReport>& issues) {
    // TODO: Implement graphics issue analysis
}

void CompatibilityChecker::analyze_audio_issues(std::vector<IssueReport>& issues) {
    // TODO: Implement audio issue analysis
}

void CompatibilityChecker::analyze_input_issues(std::vector<IssueReport>& issues) {
    // TODO: Implement input issue analysis
}

void CompatibilityChecker::analyze_save_issues(std::vector<IssueReport>& issues) {
    // TODO: Implement save issue analysis
}

void CompatibilityChecker::analyze_crash_patterns(std::vector<IssueReport>& issues) {
    const auto& session = impl->current_session;
    
    if (session.crash_count > CompatibilityCheckerImpl::MAX_CRASHES) {
        issues.push_back(IssueReport{
            .type = IssueType::Crashes,
            .description = "Frequent crashes detected",
            .workaround = "Check for system compatibility and update drivers",
            .has_workaround = true,
            .is_critical = true
        });
    }
}

bool CompatibilityChecker::is_performance_acceptable(const GameTestResult& result) {
    return result.average_fps >= CompatibilityCheckerImpl::MIN_ACCEPTABLE_FPS &&
           result.min_fps >= CompatibilityCheckerImpl::MIN_ACCEPTABLE_FPS * 0.8f;
}

bool CompatibilityChecker::are_graphics_acceptable(const GameTestResult& result) {
    for (const auto& issue : result.issues) {
        if (issue.type == IssueType::Graphics && issue.is_critical) {
            return false;
        }
    }
    return true;
}

bool CompatibilityChecker::is_stability_acceptable(const GameTestResult& result) {
    return result.crashes_count <= CompatibilityCheckerImpl::MAX_CRASHES;
}

void CompatibilityChecker::categorize_issues(
    const std::vector<IssueReport>& issues,
    std::unordered_map<IssueType, uint32_t>& counts) {
    
    for (const auto& issue : issues) {
        counts[issue.type]++;
    }
}

void CompatibilityChecker::generate_performance_report(std::string& report) {
    std::stringstream ss;
    const auto& session = impl->current_session;
    
    ss << "Performance Report:\n";
    ss << "==================\n\n";
    
    // FPS statistics
    float avg_fps = 0.0f;
    float min_fps = std::numeric_limits<float>::max();
    float max_fps = 0.0f;
    
    for (float fps : session.fps_history) {
        avg_fps += fps;
        min_fps = std::min(min_fps, fps);
        max_fps = std::max(max_fps, fps);
    }
    avg_fps /= session.fps_history.size();
    
    ss << "FPS Statistics:\n";
    ss << "- Average: " << avg_fps << "\n";
    ss << "- Minimum: " << min_fps << "\n";
    ss << "- Maximum: " << max_fps << "\n\n";
    
    // Frame time statistics
    float avg_frame_time = 0.0f;
    float max_frame_time = 0.0f;
    
    for (float frame_time : session.frame_time_history) {
        avg_frame_time += frame_time;
        max_frame_time = std::max(max_frame_time, frame_time);
    }
    avg_frame_time /= session.frame_time_history.size();
    
    ss << "Frame Time Statistics:\n";
    ss << "- Average: " << avg_frame_time << "ms\n";
    ss << "- Maximum: " << max_frame_time << "ms\n";
    
    report = ss.str();
}

void CompatibilityChecker::generate_issues_report(std::string& report) {
    std::stringstream ss;
    const auto& session = impl->current_session;
    
    ss << "Issues Report:\n";
    ss << "=============\n\n";
    
    std::unordered_map<IssueType, uint32_t> issue_counts;
    categorize_issues(session.issues, issue_counts);
    
    ss << "Issue Summary:\n";
    for (const auto& [type, count] : issue_counts) {
        ss << "- " << static_cast<int>(type) << ": " << count << " issues\n";
    }
    ss << "\n";
    
    ss << "Detailed Issues:\n";
    for (const auto& issue : session.issues) {
        ss << "- Type: " << static_cast<int>(issue.type) << "\n";
        ss << "  Description: " << issue.description << "\n";
        if (issue.has_workaround) {
            ss << "  Workaround: " << issue.workaround << "\n";
        }
        ss << "  Critical: " << (issue.is_critical ? "Yes" : "No") << "\n";
        ss << "\n";
    }
    
    report = ss.str();
}

void CompatibilityChecker::generate_compatibility_report(std::string& report) {
    std::stringstream ss;
    const auto& session = impl->current_session;
    
    ss << "Compatibility Report:\n";
    ss << "===================\n\n";
    
    ss << "Status: " << static_cast<int>(analyze_compatibility()) << "\n";
    ss << "Crashes: " << session.crash_count << "\n";
    ss << "Critical Issues: " << std::count_if(
        session.issues.begin(),
        session.issues.end(),
        [](const IssueReport& issue) { return issue.is_critical; }
    ) << "\n\n";
    
    ss << "Performance Status:\n";
    ss << "- Performance Acceptable: "
       << (is_performance_acceptable(GameTestResult{}) ? "Yes" : "No") << "\n";
    ss << "- Graphics Acceptable: "
       << (are_graphics_acceptable(GameTestResult{}) ? "Yes" : "No") << "\n";
    ss << "- Stability Acceptable: "
       << (is_stability_acceptable(GameTestResult{}) ? "Yes" : "No") << "\n";
    
    report = ss.str();
}

void CompatibilityChecker::generate_suggestions_report(std::string& report) {
    std::stringstream ss;
    
    ss << "Optimization Suggestions:\n";
    ss << "=======================\n\n";
    
    std::vector<std::string> suggestions;
    suggest_fixes(suggestions);
    
    for (const auto& suggestion : suggestions) {
        ss << "- " << suggestion << "\n";
    }
    
    if (impl->linked_profile) {
        ss << "\nProfile Optimization Suggestions:\n";
        std::vector<std::string> profile_suggestions;
        suggest_profile_optimizations(profile_suggestions);
        
        for (const auto& suggestion : profile_suggestions) {
            ss << "- " << suggestion << "\n";
        }
    }
    
    report = ss.str();
}

} // namespace compatibility 