#include "swd_launcher_port.h"
#include <android/log.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>

#define LOG_TAG "SWD_Launcher"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace fs = std::filesystem;

SwdLauncherPort& SwdLauncherPort::instance() {
    static SwdLauncherPort inst;
    return inst;
}

void SwdLauncherPort::init(const std::string& files_dir, const std::string& ext_dir) {
    m_config.files_dir = files_dir;
    m_config.external_files_dir = ext_dir;
    m_config.cache_dir = files_dir + "/Cache";

    setup_virtual_directories();
    LOGI("Mobile Launcher Port initialized. FilesDir: %s", m_config.files_dir.c_str());
}

void SwdLauncherPort::setup_virtual_directories() {
    std::vector<std::string> dirs = {
        m_config.files_dir + "/Files",
        m_config.files_dir + "/ExternalFiles",
        m_config.files_dir + "/Cache",
        m_config.files_dir + "/ExternalCache",
        m_config.files_dir + "/mods",
        m_config.files_dir + "/saves"
    };

    for (const auto& dir : dirs) {
        std::error_code ec;
        fs::create_directories(dir, ec);
        if (ec) {
            LOGE("Failed to create virtual directory %s: %s", dir.c_str(), ec.message().c_str());
        }
    }
}

void SwdLauncherPort::set_fps_cap(int fps) {
    m_config.target_fps = fps;
    m_config.unlock_fps = (fps <= 0 || fps > 60);
    LOGI("FPS Cap updated to %d (unlocked: %d)", fps, m_config.unlock_fps);
}

void SwdLauncherPort::set_mod_instance_path(const std::string& path) {
    m_config.active_mod_path = path;
    LOGI("Active Mod Instance Path set to: %s", path.c_str());
}

std::string SwdLauncherPort::get_status() const {
    if (m_config.engine_loaded) {
        return "libswordigo.so Running (Native ARM64)";
    }
    return "libswd.so Standby (Ready to launch libswordigo.so)";
}
