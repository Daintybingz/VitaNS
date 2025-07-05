#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "compatibility_checker.h"

namespace compatibility {

struct GameplaySession {
    std::string game_id;
    std::string user_id;
    uint64_t session_start;
    bool is_active;
    float current_fps;
    float current_frame_time;
    std::vector<std::string> detected_issues;
};

struct UserFeedback {
    std::string game_id;
    std::string user_id;
    std::string comment;
    int rating;  // 1-5 stars
    std::vector<std::string> tags;
    uint64_t timestamp;
};

struct GameTip {
    std::string game_id;
    std::string tip_text;
    std::string category;
    int upvotes;
    bool is_verified;
};

class CommunityHub {
public:
    CommunityHub();
    ~CommunityHub();

    // Automatic gameplay monitoring
    void start_gameplay_session(const std::string& game_id, 
                              const std::string& user_id);
    void update_gameplay_metrics(float fps, float frame_time);
    void auto_detect_issues();
    void end_gameplay_session();

    // User feedback system
    void submit_feedback(const UserFeedback& feedback);
    void report_issue(const std::string& game_id,
                     const std::string& issue_description);
    void submit_game_tip(const GameTip& tip);
    
    // Real-time optimization
    void apply_community_optimizations(const std::string& game_id);
    void suggest_real_time_fixes();
    void update_game_profile_from_community();

    // Community features
    std::vector<GameTip> get_game_tips(const std::string& game_id);
    float get_community_rating(const std::string& game_id);
    std::vector<std::string> get_recommended_settings(const std::string& game_id);
    
    // Automatic reporting
    void enable_auto_reporting(bool enabled);
    void set_metrics_callback(std::function<void(const GameplaySession&)> callback);
    void set_issue_callback(std::function<void(const std::string&)> callback);

    // Integration with compatibility checker
    void link_compatibility_checker(CompatibilityChecker* checker);
    void sync_compatibility_data();

private:
    struct CommunityHubImpl;
    std::unique_ptr<CommunityHubImpl> impl;

    // Internal helpers
    void process_gameplay_metrics();
    void update_community_database();
    void save_community_database();
    void analyze_gameplay_patterns();
    void detect_performance_issues();
    void detect_graphics_issues();
    void detect_audio_issues();
    void detect_input_issues();
    void apply_community_fixes();
    void generate_optimization_profile();
};

// Singleton accessor
CommunityHub& get_community_hub();

} // namespace compatibility 