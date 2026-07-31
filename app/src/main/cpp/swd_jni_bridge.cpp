#include "swd_jni_bridge.h"
#include "swd_launcher_port.h"
#include <android/log.h>
#include <dlfcn.h>
#include <GLES2/gl2.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

#define LOG_TAG "SWD_JNI"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// ============================================================================
// GlossHook API Definitions
// ============================================================================
typedef void (*GlossInit_t)(bool);
typedef void (*GlossEnableLog_t)(bool);
typedef void* (*GlossHook_t)(void*, void*, void**);

static GlossInit_t fn_GlossInit = nullptr;
static GlossEnableLog_t fn_GlossEnableLog = nullptr;
static GlossHook_t fn_GlossHook = nullptr;

// Orig pointers for Caver Engine Hooks
static void (*orig_CaverShell_Render)(void*, void*) = nullptr;
static void (*orig_CaverShell_Update)(void*, float) = nullptr;
static void (*orig_CaverShell_InitView)(void*) = nullptr;
static void (*orig_ProgramState_Execute)(void*, int) = nullptr;

static void hook_CaverShell_InitView(void* self) {
    LOGI("[HighPrecisionLog] CaverShell::InitView called on %p", self);
    if (orig_CaverShell_InitView) orig_CaverShell_InitView(self);
    LOGI("[HighPrecisionLog] CaverShell::InitView completed successfully!");
}

static void hook_CaverShell_Update(void* self, float dt) {
    static int count = 0;
    if (++count % 120 == 0) {
        LOGI("[HighPrecisionLog] CaverShell::Update(dt=%.4f) frame=%d self=%p", dt, count, self);
    }
    if (orig_CaverShell_Update) orig_CaverShell_Update(self, dt);
}

static void hook_CaverShell_Render(void* self, void* render_ctx) {
    static int count = 0;
    if (++count % 120 == 0) {
        LOGI("[HighPrecisionLog] CaverShell::Render(ctx=%p) frame=%d self=%p", render_ctx, count, self);
    }
    if (orig_CaverShell_Render) orig_CaverShell_Render(self, render_ctx);
}

static void hook_ProgramState_Execute(void* self, int stackIndex) {
    LOGI("[HighPrecisionLog] ProgramState::Execute(stackIndex=%d) self=%p", stackIndex, self);
    if (orig_ProgramState_Execute) orig_ProgramState_Execute(self, stackIndex);
}

// ============================================================================
// OpenGL Framebuffer Redirection for Modern GLSurfaceView (Android 10+)
// ============================================================================
static GLint s_glsurfaceview_fbo = 0;
typedef void (*fn_glBindFramebuffer)(GLenum, GLuint);
static fn_glBindFramebuffer real_glBindFramebuffer = nullptr;

extern "C" JNIEXPORT void JNICALL glBindFramebuffer(GLenum target, GLuint framebuffer) {
    if (!real_glBindFramebuffer) {
        real_glBindFramebuffer = (fn_glBindFramebuffer)dlsym(RTLD_NEXT, "glBindFramebuffer");
    }
    if (framebuffer == 0 && s_glsurfaceview_fbo != 0) {
        framebuffer = s_glsurfaceview_fbo;
    }
    if (real_glBindFramebuffer) {
        real_glBindFramebuffer(target, framebuffer);
    }
}

// ============================================================================
// Safe OpenAL Interceptor & Fallback Layer
// ============================================================================
struct ALCdevice_dummy { int id = 1; };
struct ALCcontext_dummy { int id = 1; };

static ALCdevice_dummy g_dummy_device;
static ALCcontext_dummy g_dummy_context;

static void* my_safe_alcOpenDevice(const char* devicename) {
    LOGI("[ARM64 Hook] alcOpenDevice(%s) intercepted safely!", devicename ? devicename : "NULL");
    return &g_dummy_device;
}

static void patch_memory_arm64(void* target_func, uint32_t val1, uint32_t val2 = 0) {
    if (!target_func) return;
    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)target_func & ~(page_size - 1);
    
    if (mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("mprotect PROT_WRITE failed for target %p", target_func);
        return;
    }

    uint32_t* code = (uint32_t*)target_func;
    code[0] = val1;
    if (val2 != 0) {
        code[1] = val2;
    }

    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    __builtin___clear_cache((char*)target_func, (char*)target_func + 8);
    LOGI("[ARM64 Patch] Applied memory patch at %p", target_func);
}

static void hook_arm64(void* target_func, void* replacement_func) {
    if (!target_func || !replacement_func) return;
    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t page_start = (uintptr_t)target_func & ~(page_size - 1);
    
    if (mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        LOGE("mprotect PROT_WRITE failed for target %p", target_func);
        return;
    }

    uint32_t* code = (uint32_t*)target_func;
    code[0] = 0x58000050;
    code[1] = 0xD61F0200;
    *(uint64_t*)&code[2] = (uint64_t)replacement_func;

    mprotect((void*)page_start, page_size * 2, PROT_READ | PROT_EXEC);
    __builtin___clear_cache((char*)target_func, (char*)target_func + 16);
    LOGI("[ARM64 Hook] Successfully hooked target %p -> %p", target_func, replacement_func);
}

extern "C" {

JNIEXPORT void* JNICALL alcOpenDevice(const char* devicename) {
    return my_safe_alcOpenDevice(devicename);
}

JNIEXPORT void* JNICALL alcCreateContext(void* device, const int* attrlist) {
    LOGI("[OpenAL Interceptor] alcCreateContext(%p) called", device);
    return &g_dummy_context;
}

JNIEXPORT unsigned char JNICALL alcMakeContextCurrent(void* context) {
    LOGI("[OpenAL Interceptor] alcMakeContextCurrent(%p) called", context);
    return 1;
}

JNIEXPORT void JNICALL alcProcessContext(void* context) {}
JNIEXPORT void JNICALL alcSuspendContext(void* context) {}
JNIEXPORT void JNICALL alcDestroyContext(void* context) {}
JNIEXPORT unsigned char JNICALL alcCloseDevice(void* device) { return 1; }
JNIEXPORT void* JNICALL alcGetCurrentContext() { return &g_dummy_context; }

} // extern "C"

// ============================================================================
// Native JNI Bridge Implementation
// ============================================================================

SwdJniBridge& SwdJniBridge::instance() {
    static SwdJniBridge inst;
    return inst;
}

bool SwdJniBridge::load_swordigo_library(const std::string& lib_path) {
    LOGI("Pre-loading libopenal-soft.so...");
    void* openal_handle = dlopen("libopenal-soft.so", RTLD_NOW | RTLD_GLOBAL);
    if (openal_handle) {
        LOGI("libopenal-soft.so loaded! Installing ARM64 inline hook for alcOpenDevice...");
        void* alc_target = dlsym(openal_handle, "alcOpenDevice");
        if (alc_target) {
            hook_arm64(alc_target, (void*)my_safe_alcOpenDevice);
        } else {
            LOGE("Failed to find alcOpenDevice symbol in libopenal-soft.so");
        }
    } else {
        LOGE("Failed to dlopen libopenal-soft.so: %s", dlerror());
    }

    LOGI("Pre-loading libGlossHook.so...");
    void* gloss_handle = dlopen("libGlossHook.so", RTLD_NOW | RTLD_GLOBAL);
    if (!gloss_handle) {
        LOGI("libGlossHook.so not found: %s", dlerror());
    } else {
        LOGI("libGlossHook.so loaded! Resolving GlossHook symbols...");
        fn_GlossInit = (GlossInit_t)dlsym(gloss_handle, "GlossInit");
        fn_GlossEnableLog = (GlossEnableLog_t)dlsym(gloss_handle, "GlossEnableLog");
        fn_GlossHook = (GlossHook_t)dlsym(gloss_handle, "GlossHook");
        if (fn_GlossInit) fn_GlossInit(false);
        if (fn_GlossEnableLog) fn_GlossEnableLog(true);
        LOGI("GlossHook initialized successfully!");
    }

    LOGI("Attempting to load libswordigo.so from path: %s", lib_path.c_str());
    m_swordigo_handle = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!m_swordigo_handle) {
        LOGI("Path dlopen failed (%s). Trying system native lib load for 'libswordigo.so'...", dlerror());
        m_swordigo_handle = dlopen("libswordigo.so", RTLD_NOW | RTLD_GLOBAL);
    }

    if (!m_swordigo_handle) {
        LOGE("Failed to load libswordigo.so: %s", dlerror());
        return false;
    }

    LOGI("libswordigo.so loaded successfully! Installing GlossHooks & Patches...");

    // Apply GlossHooks for deep engine logging
    if (fn_GlossHook) {
        void* sym_initView = dlsym(m_swordigo_handle, "_ZN5Caver10CaverShell8InitViewEv");
        if (sym_initView) fn_GlossHook(sym_initView, (void*)hook_CaverShell_InitView, (void**)&orig_CaverShell_InitView);

        void* sym_update = dlsym(m_swordigo_handle, "_ZN5Caver10CaverShell6UpdateEf");
        if (sym_update) fn_GlossHook(sym_update, (void*)hook_CaverShell_Update, (void**)&orig_CaverShell_Update);

        void* sym_render = dlsym(m_swordigo_handle, "_ZN5Caver10CaverShell6RenderEPNS_16RenderingContextE");
        if (sym_render) fn_GlossHook(sym_render, (void*)hook_CaverShell_Render, (void**)&orig_CaverShell_Render);

        void* sym_exec = dlsym(m_swordigo_handle, "_ZN5Caver12ProgramState7ExecuteEi");
        if (sym_exec) fn_GlossHook(sym_exec, (void*)hook_ProgramState_Execute, (void**)&orig_ProgramState_Execute);
    }

    // Patch Store & Ads
    void* noAdsCheck = dlsym(m_swordigo_handle, "_ZN5Caver15StoreController20IsNoAdsUnlockedCheckEv");
    if (noAdsCheck) patch_memory_arm64(noAdsCheck, 0x52800020, 0xD65F03C0);

    void* isPurchased = dlsym(m_swordigo_handle, "_ZN5Caver23StoreController_Android18IsProductPurchasedERKSs");
    if (!isPurchased) isPurchased = dlsym(m_swordigo_handle, "_ZN5Caver15StoreController18IsProductPurchasedERKSs");
    if (isPurchased) patch_memory_arm64(isPurchased, 0x52800020, 0xD65F03C0);

    void* showAd = dlsym(m_swordigo_handle, "_ZN5Caver24OnlineController_Android18ShowInterstitialAdERKSsif");
    if (showAd) patch_memory_arm64(showAd, 0xD65F03C0);

    void* showAd2 = dlsym(m_swordigo_handle, "_ZN5Caver25AndroidShowInterstitialAdEd");
    if (showAd2) patch_memory_arm64(showAd2, 0xD65F03C0);

    // Patch Google Game Services check -> return false (MOV W0, #0; RET)
    void* gservices = dlsym(m_swordigo_handle, "_ZN5Caver36AndroidIsGoogleGameServicesAvailableEv");
    if (gservices) {
        patch_memory_arm64(gservices, 0x52800000, 0xD65F03C0);
        LOGI("[Patch] Applied AndroidIsGoogleGameServicesAvailable -> return false");
    }

    // Patch HandleGoogleSignInCompleted -> RET (0xD65F03C0)
    void* handleSignIn = dlsym(m_swordigo_handle, "_ZN5Caver24OnlineController_Android27HandleGoogleSignInCompletedEb");
    if (handleSignIn) {
        patch_memory_arm64(handleSignIn, 0xD65F03C0);
        LOGI("[Patch] Applied HandleGoogleSignInCompleted -> RET");
    }

    // Patch boost::throw_exception<boost::bad_weak_ptr> -> RET (0xD65F03C0)
    void* throwBadWeak = dlsym(m_swordigo_handle, "_ZN5boost15throw_exceptionINS_12bad_weak_ptrEEEvRKT_");
    if (throwBadWeak) {
        patch_memory_arm64(throwBadWeak, 0xD65F03C0);
        LOGI("[Patch] Applied boost::throw_exception<bad_weak_ptr> -> RET");
    }

    LOGI("Resolving JNI native symbols...");
    resolve_swordigo_symbols();
    SwdLauncherPort::instance().config().engine_loaded = true;
    return true;
}

void SwdJniBridge::resolve_swordigo_symbols() {
    if (!m_swordigo_handle) return;

#define RESOLVE_SYM(fn_type, var, name) \
    var = (fn_type)dlsym(m_swordigo_handle, name); \
    if (!var) { LOGE("Failed to resolve symbol: %s", name); } \
    else { LOGI("Resolved symbol: %s -> %p", name, (void*)var); }

    RESOLVE_SYM(fn_setupNativeInterface, p_setupNativeInterface, "Java_com_touchfoo_swordigo_Native_setupNativeInterface");
    RESOLVE_SYM(fn_setFilesDir, p_setFilesDir, "Java_com_touchfoo_swordigo_Native_setFilesDir");
    RESOLVE_SYM(fn_setCacheDir, p_setCacheDir, "Java_com_touchfoo_swordigo_Native_setCacheDir");
    RESOLVE_SYM(fn_setAssetManager, p_setAssetManager, "Java_com_touchfoo_swordigo_Native_setAssetManager");
    RESOLVE_SYM(fn_setupApplication, p_setupApplication, "Java_com_touchfoo_swordigo_Native_setupApplication");
    RESOLVE_SYM(fn_handleApplicationLaunch, p_handleApplicationLaunch, "Java_com_touchfoo_swordigo_Native_handleApplicationLaunch");
    RESOLVE_SYM(fn_setApplicationViewSize, p_setApplicationViewSize, "Java_com_touchfoo_swordigo_Native_setApplicationViewSize");
    RESOLVE_SYM(fn_updateApplication, p_updateApplication, "Java_com_touchfoo_swordigo_Native_updateApplication");
    RESOLVE_SYM(fn_drawApplication, p_drawApplication, "Java_com_touchfoo_swordigo_Native_drawApplication");
    RESOLVE_SYM(fn_handleTouchEvent, p_handleTouchEvent, "Java_com_touchfoo_swordigo_Native_handleTouchEvent");
    RESOLVE_SYM(fn_applicationDidBecomeActive, p_applicationDidBecomeActive, "Java_com_touchfoo_swordigo_Native_applicationDidBecomeActive");
    RESOLVE_SYM(fn_applicationDidEnterBackground, p_applicationDidEnterBackground, "Java_com_touchfoo_swordigo_Native_applicationDidEnterBackground");
    RESOLVE_SYM(fn_applicationDidEnterForeground, p_applicationDidEnterForeground, "Java_com_touchfoo_swordigo_Native_applicationDidEnterForeground");

#undef RESOLVE_SYM
}

void SwdJniBridge::init_engine_context(JNIEnv* env, const std::string& files_dir, const std::string& cache_dir, jobject asset_mgr_obj, int w, int h) {
    if (!env || !m_swordigo_handle) return;

    m_files_dir = files_dir;
    m_cache_dir = cache_dir;
    m_view_w = w > 0 ? w : 1280;
    m_view_h = h > 0 ? h : 720;

    LOGI("[Engine Storage Configured] FilesDir: %s, CacheDir: %s", m_files_dir.c_str(), m_cache_dir.c_str());

    if (p_setFilesDir) {
        jstring jfiles = env->NewStringUTF(files_dir.c_str());
        p_setFilesDir(env, nullptr, jfiles);
        env->DeleteLocalRef(jfiles);
    }
    if (p_setCacheDir) {
        jstring jcache = env->NewStringUTF(cache_dir.c_str());
        p_setCacheDir(env, nullptr, jcache);
        env->DeleteLocalRef(jcache);
    }
    if (p_setAssetManager && asset_mgr_obj) {
        p_setAssetManager(env, nullptr, asset_mgr_obj);
    }
}

void SwdJniBridge::on_surface_created() {
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    m_last_frame_time = std::chrono::high_resolution_clock::now();
    LOGI("[GLThread] Surface Created - Ready for OpenGL setup");
}

void SwdJniBridge::on_surface_changed(JNIEnv* env, int w, int h) {
    glViewport(0, 0, w, h);
    m_view_w = w;
    m_view_h = h;
    LOGI("[GLThread] Surface Viewport Changed: %dx%d", w, h);

    if (!m_app_launched && env) {
        LOGI("[GLThread] Initializing engine OpenGL context on GLThread...");
        if (p_handleApplicationLaunch) {
            p_handleApplicationLaunch(env, nullptr);
        }
        if (p_setupNativeInterface) {
            p_setupNativeInterface(env, nullptr);
        }
        if (p_setupApplication) {
            p_setupApplication(env, nullptr);
        }
        if (p_setApplicationViewSize) {
            p_setApplicationViewSize(env, nullptr, 1280, 720, JNI_TRUE, 320, 1);
        }
        if (p_applicationDidBecomeActive) {
            p_applicationDidBecomeActive(env, nullptr);
        }
        m_app_launched = true;
        LOGI("[GLThread] Engine OpenGL context successfully initialized & active!");
    } else if (p_setApplicationViewSize && env) {
        p_setApplicationViewSize(env, nullptr, 1280, 720, JNI_TRUE, 320, 1);
    }
}

void SwdJniBridge::on_draw_frame(JNIEnv* env) {
    auto now = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float>(now - m_last_frame_time).count();
    m_last_frame_time = now;

    if (dt > 0.1f) dt = 0.1f;

    // Record GLSurfaceView's bound FBO ID so glBindFramebuffer(0) gets redirected to it!
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s_glsurfaceview_fbo);

    static int s_frame_count = 0;
    if (++s_frame_count % 120 == 0) {
        LOGI("[GLThread] Frame %d - dt=%.4f app_launched=%d fbo=%d p_draw=%p", s_frame_count, dt, m_app_launched, s_glsurfaceview_fbo, (void*)p_drawApplication);
    }

    if (m_app_launched && env) {
        if (p_updateApplication) {
            p_updateApplication(env, nullptr, dt);
        }
        if (p_drawApplication) {
            p_drawApplication(env, nullptr);
        }
    }
}

void SwdJniBridge::on_touch_event(JNIEnv* env, int action, float x, float y, int pointer_id) {
    if (m_app_launched && p_handleTouchEvent && env) {
        double timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        p_handleTouchEvent(env, nullptr, action, 1, timestamp, x, y, m_last_touch_x, m_last_touch_y, pointer_id);
        m_last_touch_x = x;
        m_last_touch_y = y;
    }
}
