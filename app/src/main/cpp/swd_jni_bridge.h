#ifndef SWD_JNI_BRIDGE_H
#define SWD_JNI_BRIDGE_H

#include <jni.h>
#include <android/asset_manager.h>
#include <string>
#include <chrono>

typedef void (*fn_setupNativeInterface)(JNIEnv*, jclass);
typedef void (*fn_setFilesDir)(JNIEnv*, jclass, jstring);
typedef void (*fn_setCacheDir)(JNIEnv*, jclass, jstring);
typedef void (*fn_setAssetManager)(JNIEnv*, jclass, jobject);
typedef void (*fn_setupApplication)(JNIEnv*, jclass);
typedef void (*fn_handleApplicationLaunch)(JNIEnv*, jclass);
typedef void (*fn_setApplicationViewSize)(JNIEnv*, jclass, jint, jint, jboolean, jint, jint);
typedef void (*fn_updateApplication)(JNIEnv*, jclass, jfloat);
typedef void (*fn_drawApplication)(JNIEnv*, jclass);
typedef void (*fn_handleTouchEvent)(JNIEnv*, jclass, jint, jint, jdouble, jfloat, jfloat, jfloat, jfloat, jint);
typedef void (*fn_applicationDidBecomeActive)(JNIEnv*, jclass);
typedef void (*fn_applicationDidEnterBackground)(JNIEnv*, jclass);
typedef void (*fn_applicationDidEnterForeground)(JNIEnv*, jclass);

class SwdJniBridge {
public:
    static SwdJniBridge& instance();

    bool load_swordigo_library(const std::string& lib_path);
    void resolve_swordigo_symbols();
    void init_engine_context(JNIEnv* env, const std::string& files_dir, const std::string& cache_dir, jobject asset_mgr_obj, int w, int h);

    // Engine lifecycle triggers
    void on_surface_created();
    void on_surface_changed(JNIEnv* env, int w, int h);
    void on_draw_frame(JNIEnv* env);
    void on_touch_event(JNIEnv* env, int action, float x, float y, int pointer_id);

    void set_asset_manager(AAssetManager* mgr) { m_asset_mgr = mgr; }
    AAssetManager* get_asset_manager() const { return m_asset_mgr; }

    bool is_loaded() const { return m_swordigo_handle != nullptr; }

private:
    SwdJniBridge() = default;
    void* m_swordigo_handle = nullptr;
    AAssetManager* m_asset_mgr = nullptr;

    // Resolved function pointers from libswordigo.so
    fn_setupNativeInterface p_setupNativeInterface = nullptr;
    fn_setFilesDir p_setFilesDir = nullptr;
    fn_setCacheDir p_setCacheDir = nullptr;
    fn_setAssetManager p_setAssetManager = nullptr;
    fn_setupApplication p_setupApplication = nullptr;
    fn_handleApplicationLaunch p_handleApplicationLaunch = nullptr;
    fn_setApplicationViewSize p_setApplicationViewSize = nullptr;
    fn_updateApplication p_updateApplication = nullptr;
    fn_drawApplication p_drawApplication = nullptr;
    fn_handleTouchEvent p_handleTouchEvent = nullptr;
    fn_applicationDidBecomeActive p_applicationDidBecomeActive = nullptr;
    fn_applicationDidEnterBackground p_applicationDidEnterBackground = nullptr;
    fn_applicationDidEnterForeground p_applicationDidEnterForeground = nullptr;

    std::chrono::high_resolution_clock::time_point m_last_frame_time;
    bool m_app_launched = false;
    std::string m_files_dir;
    std::string m_cache_dir;
    int m_view_w = 1280;
    int m_view_h = 720;
    float m_last_touch_x = 0.0f;
    float m_last_touch_y = 0.0f;
};

#endif // SWD_JNI_BRIDGE_H
