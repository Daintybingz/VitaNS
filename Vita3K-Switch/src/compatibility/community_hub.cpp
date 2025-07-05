#include "community_hub.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <json.hpp>

namespace compatibility {

using json = nlohmann::json;

void to_json(nlohmann::json& j, const UserFeedback& f) {
    j = nlohmann::json{
        {"game_id", f.game_id},
        {"user_id", f.user_id},
        {"comment", f.comment},
        {"rating", f.rating},
        {"tags", f.tags},
        {"timestamp", f.timestamp}
    };
}
void to_json(nlohmann::json& j, const GameTip& t) {
    j = nlohmann::json{
        {"game_id", t.game_id},
        {"tip_text", t.tip_text},
        {"category", t.category},
        {"upvotes", t.upvotes},
        {"is_verified", t.is_verified}
    };
}

struct CommunityHub::CommunityHubImpl {
    // Active session
    GameplaySession current_session;
    std::mutex session_mutex;
    
    // Callbacks
    std::function<void(const GameplaySession&)> metrics_callback;
    std::function<void(const std::string&)> issue_callback;
    
    // Settings
    bool auto_reporting_enabled = true;
    uint32_t metrics_interval_ms = 1000;
    uint32_t issue_detection_interval_ms = 5000;
    
    // Database
    std::unordered_map<std::string, std::vector<UserFeedback>> feedback_database;
    std::unordered_map<std::string, std::vector<GameTip>> tips_database;
    std::unordered_map<std::string, float> community_ratings;
    std::unordered_map<std::string, std::vector<std::string>> recommended_settings;
    
    // Performance tracking
    struct PerformanceMetrics {
        std::vector<float> fps_history;
        std::vector<float> frame_time_history;
        float avg_fps = 0.0f;
        float avg_frame_time = 0.0f;
        uint32_t stutter_count = 0;
    };
    std::unordered_map<std::string, PerformanceMetrics> performance_data;
    
    // Issue detection
    struct IssueDetection {
        bool checking_enabled = true;
        float fps_threshold = 25.0f;
        float stutter_threshold_ms = 33.33f;
        uint32_t required_samples = 60;
    } issue_detection;
    
    // Integration
    CompatibilityChecker* compatibility_checker = nullptr;
    
    // Background worker
    std::thread metrics_worker;
    std::thread issue_detector;
    bool should_stop = false;
};

// Singleton instance
static std::unique_ptr<CommunityHub> g_instance;
static std::once_flag g_init_flag;

CommunityHub& get_community_hub() {
    std::call_once(g_init_flag, []() {
        g_instance = std::make_unique<CommunityHub>();
    });
    return *g_instance;
}

CommunityHub::CommunityHub() : impl(std::make_unique<CommunityHubImpl>()) {
    // Start background workers
    impl->metrics_worker = std::thread([this]() {
        while (!impl->should_stop) {
            process_gameplay_metrics();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(impl->metrics_interval_ms));
        }
    });
    
    impl->issue_detector = std::thread([this]() {
        while (!impl->should_stop) {
            if (impl->issue_detection.checking_enabled) {
                auto_detect_issues();
            }
            std::this_thread::sleep_for(
                std::chrono::milliseconds(impl->issue_detection_interval_ms));
        }
    });
}

CommunityHub::~CommunityHub() {
    impl->should_stop = true;
    if (impl->metrics_worker.joinable()) {
        impl->metrics_worker.join();
    }
    if (impl->issue_detector.joinable()) {
        impl->issue_detector.join();
    }
}

void CommunityHub::start_gameplay_session(const std::string& game_id,
                                        const std::string& user_id) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    impl->current_session = GameplaySession{
        .game_id = game_id,
        .user_id = user_id,
        .session_start = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count()),
        .is_active = true
    };
    
    // Apply community optimizations automatically
    apply_community_optimizations(game_id);
    
    // Start collecting metrics
    impl->performance_data[game_id] = CommunityHubImpl::PerformanceMetrics{};
    
    // Notify compatibility checker
    if (impl->compatibility_checker) {
        impl->compatibility_checker->start_test_session(game_id, user_id);
    }
}

void CommunityHub::update_gameplay_metrics(float fps, float frame_time) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    if (!impl->current_session.is_active) return;
    
    // Update current session
    impl->current_session.current_fps = fps;
    impl->current_session.current_frame_time = frame_time;
    
    // Update performance tracking
    auto& metrics = impl->performance_data[impl->current_session.game_id];
    metrics.fps_history.push_back(fps);
    metrics.frame_time_history.push_back(frame_time);
    
    // Keep reasonable history size
    const size_t max_history = 300; // 5 minutes at 60 fps
    if (metrics.fps_history.size() > max_history) {
        metrics.fps_history.erase(metrics.fps_history.begin());
        metrics.frame_time_history.erase(metrics.frame_time_history.begin());
    }
    
    // Update averages
    metrics.avg_fps = 0.0f;
    metrics.avg_frame_time = 0.0f;
    for (size_t i = 0; i < metrics.fps_history.size(); i++) {
        metrics.avg_fps += metrics.fps_history[i];
        metrics.avg_frame_time += metrics.frame_time_history[i];
    }
    metrics.avg_fps /= metrics.fps_history.size();
    metrics.avg_frame_time /= metrics.frame_time_history.size();
    
    // Detect stutters
    if (frame_time > impl->issue_detection.stutter_threshold_ms) {
        metrics.stutter_count++;
    }
    
    // Update compatibility checker
    if (impl->compatibility_checker) {
        impl->compatibility_checker->update_performance_metrics(fps, frame_time);
    }
}

void CommunityHub::auto_detect_issues() {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    if (!impl->current_session.is_active) return;
    
    const auto& game_id = impl->current_session.game_id;
    const auto& metrics = impl->performance_data[game_id];
    
    // Only analyze if we have enough samples
    if (metrics.fps_history.size() < impl->issue_detection.required_samples) {
        return;
    }
    
    // Clear previous detections
    impl->current_session.detected_issues.clear();
    
    // Performance issues
    if (metrics.avg_fps < impl->issue_detection.fps_threshold) {
        impl->current_session.detected_issues.push_back(
            "Performance: Low average FPS (" + 
            std::to_string(static_cast<int>(metrics.avg_fps)) + " FPS)");
    }
    
    // Stutter detection
    if (metrics.stutter_count > 0) {
        impl->current_session.detected_issues.push_back(
            "Performance: Detected " + 
            std::to_string(metrics.stutter_count) + " frame time spikes");
    }
    
    // Notify via callback
    if (!impl->current_session.detected_issues.empty() && impl->issue_callback) {
        for (const auto& issue : impl->current_session.detected_issues) {
            impl->issue_callback(issue);
        }
    }
    
    // Report to compatibility checker
    if (impl->compatibility_checker) {
        for (const auto& issue : impl->current_session.detected_issues) {
            impl->compatibility_checker->record_issue(IssueReport{
                .type = IssueType::Performance,
                .description = issue,
                .has_workaround = true,
                .is_critical = false
            });
        }
    }
}

void CommunityHub::end_gameplay_session() {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    if (!impl->current_session.is_active) return;
    
    // Update community database
    update_community_database();
    
    // End compatibility checker session
    if (impl->compatibility_checker) {
        impl->compatibility_checker->end_test_session();
    }
    
    // Clear session
    impl->current_session.is_active = false;
}

void CommunityHub::submit_feedback(const UserFeedback& feedback) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    impl->feedback_database[feedback.game_id].push_back(feedback);
    
    // Update community rating
    auto& ratings = impl->feedback_database[feedback.game_id];
    float total = 0.0f;
    for (const auto& fb : ratings) {
        total += fb.rating;
    }
    impl->community_ratings[feedback.game_id] = total / ratings.size();
    
    // Sync with compatibility checker
    sync_compatibility_data();
}

void CommunityHub::report_issue(const std::string& game_id,
                              const std::string& issue_description) {
    if (impl->compatibility_checker) {
        impl->compatibility_checker->record_issue(IssueReport{
            .type = IssueType::Other,
            .description = issue_description,
            .has_workaround = false,
            .is_critical = false
        });
    }
}

void CommunityHub::submit_game_tip(const GameTip& tip) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    impl->tips_database[tip.game_id].push_back(tip);
}

void CommunityHub::apply_community_optimizations(const std::string& game_id) {
    // Get recommended settings
    auto it = impl->recommended_settings.find(game_id);
    if (it == impl->recommended_settings.end()) return;
    
    // Apply settings through compatibility checker
    if (impl->compatibility_checker) {
        for (const auto& setting : it->second) {
            // TODO: Apply setting through game profile
        }
    }
}

void CommunityHub::suggest_real_time_fixes() {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    if (!impl->current_session.is_active) return;
    
    const auto& game_id = impl->current_session.game_id;
    const auto& metrics = impl->performance_data[game_id];
    
    std::vector<std::string> suggestions;
    
    // Performance suggestions
    if (metrics.avg_fps < impl->issue_detection.fps_threshold) {
        suggestions.push_back("Consider lowering graphics settings");
        suggestions.push_back("Check recommended community settings");
    }
    
    // Get community tips
    auto tips = get_game_tips(game_id);
    for (const auto& tip : tips) {
        if (tip.is_verified) {
            suggestions.push_back(tip.tip_text);
        }
    }
    
    // Notify via callback
    if (impl->issue_callback) {
        for (const auto& suggestion : suggestions) {
            impl->issue_callback("Suggestion: " + suggestion);
        }
    }
}

void CommunityHub::update_game_profile_from_community() {
    if (!impl->compatibility_checker) return;
    
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    const auto& game_id = impl->current_session.game_id;
    
    // Get community recommendations
    auto it = impl->recommended_settings.find(game_id);
    if (it == impl->recommended_settings.end()) return;
    
    // Update game profile
    GameTestResult result;
    result.game_id = game_id;
    result.status = CompatibilityStatus::Unknown;
    
    // Add community-reported issues
    for (const auto& feedback : impl->feedback_database[game_id]) {
        for (const auto& tag : feedback.tags) {
            result.issues.push_back(IssueReport{
                .type = IssueType::Other,
                .description = tag,
                .has_workaround = false,
                .is_critical = false
            });
        }
    }
    
    impl->compatibility_checker->update_game_profile(result);
}

std::vector<GameTip> CommunityHub::get_game_tips(const std::string& game_id) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    auto it = impl->tips_database.find(game_id);
    if (it == impl->tips_database.end()) {
        return {};
    }
    
    // Sort by upvotes
    auto tips = it->second;
    std::sort(tips.begin(), tips.end(),
        [](const GameTip& a, const GameTip& b) {
            return a.upvotes > b.upvotes;
        });
    
    return tips;
}

float CommunityHub::get_community_rating(const std::string& game_id) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    auto it = impl->community_ratings.find(game_id);
    return it != impl->community_ratings.end() ? it->second : 0.0f;
}

std::vector<std::string> CommunityHub::get_recommended_settings(
    const std::string& game_id) {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    auto it = impl->recommended_settings.find(game_id);
    return it != impl->recommended_settings.end() ? it->second : 
        std::vector<std::string>{};
}

void CommunityHub::enable_auto_reporting(bool enabled) {
    impl->auto_reporting_enabled = enabled;
}

void CommunityHub::set_metrics_callback(
    std::function<void(const GameplaySession&)> callback) {
    impl->metrics_callback = callback;
}

void CommunityHub::set_issue_callback(
    std::function<void(const std::string&)> callback) {
    impl->issue_callback = callback;
}

void CommunityHub::link_compatibility_checker(CompatibilityChecker* checker) {
    impl->compatibility_checker = checker;
}

void CommunityHub::sync_compatibility_data() {
    if (!impl->compatibility_checker) return;
    
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    // Sync all game data
    for (const auto& [game_id, feedback_list] : impl->feedback_database) {
        GameTestResult result;
        result.game_id = game_id;
        
        // Calculate average rating
        float total_rating = 0.0f;
        for (const auto& feedback : feedback_list) {
            total_rating += feedback.rating;
        }
        float avg_rating = total_rating / feedback_list.size();
        
        // Map rating to compatibility status
        if (avg_rating >= 4.5f) {
            result.status = CompatibilityStatus::Perfect;
        } else if (avg_rating >= 4.0f) {
            result.status = CompatibilityStatus::Playable;
        } else if (avg_rating >= 3.0f) {
            result.status = CompatibilityStatus::InGame;
        } else if (avg_rating >= 2.0f) {
            result.status = CompatibilityStatus::Menu;
        } else {
            result.status = CompatibilityStatus::Boots;
        }
        
        impl->compatibility_checker->update_test_result(result);
    }
}

// Private methods

void CommunityHub::process_gameplay_metrics() {
    std::lock_guard<std::mutex> lock(impl->session_mutex);
    
    if (!impl->current_session.is_active) return;
    
    // Process current metrics
    if (impl->metrics_callback) {
        impl->metrics_callback(impl->current_session);
    }
    
    // Update community database periodically
    static uint32_t update_counter = 0;
    if (++update_counter >= 60) { // Update every ~60 seconds
        update_community_database();
        update_counter = 0;
    }
}

void CommunityHub::update_community_database() {
    if (!impl->current_session.is_active) return;
    
    const auto& game_id = impl->current_session.game_id;
    const auto& metrics = impl->performance_data[game_id];
    
    // Update recommended settings based on performance
    if (metrics.avg_fps >= 30.0f && metrics.stutter_count == 0) {
        // This configuration works well
        auto current_settings = get_recommended_settings(game_id);
        // TODO: Get current graphics/emulation settings
        // current_settings.push_back(current_config);
        impl->recommended_settings[game_id] = current_settings;
    }
    
    // Save to disk periodically
    save_community_database();
}

void CommunityHub::analyze_gameplay_patterns() {
    if (!impl->current_session.is_active) return;
    
    const auto& game_id = impl->current_session.game_id;
    const auto& metrics = impl->performance_data[game_id];
    
    // Analyze performance patterns
    if (metrics.fps_history.size() >= impl->issue_detection.required_samples) {
        // TODO: Implement pattern analysis
    }
}

void CommunityHub::detect_performance_issues() {
    if (!impl->current_session.is_active) return;
    
    const auto& metrics = impl->performance_data[impl->current_session.game_id];
    
    if (metrics.avg_fps < impl->issue_detection.fps_threshold) {
        suggest_real_time_fixes();
    }
}

void CommunityHub::detect_graphics_issues() {
    // TODO: Implement graphics issue detection
}

void CommunityHub::detect_audio_issues() {
    // TODO: Implement audio issue detection
}

void CommunityHub::detect_input_issues() {
    // TODO: Implement input issue detection
}

void CommunityHub::apply_community_fixes() {
    if (!impl->current_session.is_active) return;
    
    const auto& game_id = impl->current_session.game_id;
    
    // Get verified community fixes
    auto tips = get_game_tips(game_id);
    for (const auto& tip : tips) {
        if (tip.is_verified) {
            // TODO: Apply verified fixes automatically
        }
    }
}

void CommunityHub::generate_optimization_profile() {
    if (!impl->current_session.is_active) return;
    
    const auto& game_id = impl->current_session.game_id;
    const auto& metrics = impl->performance_data[game_id];
    
    // Generate profile based on performance data
    if (metrics.fps_history.size() >= impl->issue_detection.required_samples) {
        // TODO: Generate optimization profile
    }
}

void CommunityHub::save_community_database() {
    try {
        json j;
        
        // Save feedback
        for (const auto& [key, vec] : impl->feedback_database) {
            j["feedback"][key] = nlohmann::json::array();
            for (const auto& item : vec) {
                j["feedback"][key].push_back(nlohmann::json(item));
            }
        }
        // Save tips
        for (const auto& [key, vec] : impl->tips_database) {
            j["tips"][key] = nlohmann::json::array();
            for (const auto& item : vec) {
                j["tips"][key].push_back(nlohmann::json(item));
            }
        }
        
        // Save ratings
        j["ratings"] = impl->community_ratings;
        
        // Save recommended settings
        j["settings"] = impl->recommended_settings;
        
        // Save to file
        std::ofstream file("community_data.json");
        file << j.dump(4);
    } catch (const std::exception& e) {
        // Handle saving errors
    }
}

} // namespace compatibility 