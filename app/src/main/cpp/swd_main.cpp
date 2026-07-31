#include <jni.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include "swd_jni_bridge.h"
#include "swd_launcher_port.h"

#define LOG_TAG "SWD_Main"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static JavaVM* g_jvm = nullptr;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    LOGI("libswd.so JNI_OnLoad initialized successfully!");
    return JNI_VERSION_1_6;
}

extern "C" {

JNIEXPORT jboolean JNICALL
Java_com_swordigo_desktop_NativeBridge_initNativeEngine(JNIEnv *env, jclass clazz,
                                                         jstring filesDir, jstring extFilesDir,
                                                         jobject assetManager) {
    const char *native_files_dir = env->GetStringUTFChars(filesDir, nullptr);
    const char *native_ext_dir = env->GetStringUTFChars(extFilesDir, nullptr);

    SwdLauncherPort::instance().init(native_files_dir, native_ext_dir);

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    SwdJniBridge::instance().set_asset_manager(mgr);

    // Attempt to load libswordigo.so
    std::string lib_path = std::string(native_files_dir) + "/libswordigo.so";
    bool loaded = SwdJniBridge::instance().load_swordigo_library(lib_path);

    if (loaded) {
        SwdJniBridge::instance().init_engine_context(env, native_files_dir, native_files_dir + std::string("/Cache"), assetManager, 1280, 720);
    }

    env->ReleaseStringUTFChars(filesDir, native_files_dir);
    env->ReleaseStringUTFChars(extFilesDir, native_ext_dir);

    return loaded ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_onSurfaceCreated(JNIEnv *env, jclass clazz) {
    SwdJniBridge::instance().on_surface_created();
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_onSurfaceChanged(JNIEnv *env, jclass clazz,
                                                         jint width, jint height) {
    SwdJniBridge::instance().on_surface_changed(env, width, height);
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_onDrawFrame(JNIEnv *env, jclass clazz) {
    SwdJniBridge::instance().on_draw_frame(env);
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_onTouchEvent(JNIEnv *env, jclass clazz,
                                                     jint action, jfloat x, jfloat y,
                                                     jint pointer_id) {
    SwdJniBridge::instance().on_touch_event(env, action, x, y, pointer_id);
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_onPause(JNIEnv *env, jclass clazz) {
    LOGI("Engine Paused.");
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_onResume(JNIEnv *env, jclass clazz) {
    LOGI("Engine Resumed.");
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_setModInstancePath(JNIEnv *env, jclass clazz,
                                                           jstring path) {
    const char *native_path = env->GetStringUTFChars(path, nullptr);
    SwdLauncherPort::instance().set_mod_instance_path(native_path);
    env->ReleaseStringUTFChars(path, native_path);
}

JNIEXPORT void JNICALL
Java_com_swordigo_desktop_NativeBridge_setFpsCap(JNIEnv *env, jclass clazz, jint fps) {
    SwdLauncherPort::instance().set_fps_cap(fps);
}

JNIEXPORT jstring JNICALL
Java_com_swordigo_desktop_NativeBridge_getEngineStatus(JNIEnv *env, jclass clazz) {
    std::string status = SwdLauncherPort::instance().get_status();
    return env->NewStringUTF(status.c_str());
}

} // extern "C"
