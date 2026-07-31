#ifndef SWD_LAUNCHER_PORT_H
#define SWD_LAUNCHER_PORT_H

#include <string>
#include <vector>

struct MobileLauncherConfig {
    std::string files_dir;
    std::string external_files_dir;
    std::string cache_dir;
    std::string active_mod_path;
    int target_fps = 60;
    bool unlock_fps = false;
    bool enable_mod_support = true;
    bool engine_loaded = false;
};

class SwdLauncherPort {
public:
    static SwdLauncherPort& instance();

    void init(const std::string& files_dir, const std::string& ext_dir);
    void setup_virtual_directories();
    void set_fps_cap(int fps);
    void set_mod_instance_path(const std::string& path);
    std::string get_status() const;

    MobileLauncherConfig& config() { return m_config; }

private:
    SwdLauncherPort() = default;
    MobileLauncherConfig m_config;
};

#endif // SWD_LAUNCHER_PORT_H
