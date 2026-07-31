# SwordigoMobile (Swordfare)

> **Maintained by MrSinup**  
> **Experimental Mobile Port & Custom Engine Host for Swordigo**

---

> [!WARNING]  
> **EXPERIMENTAL PROJECT NOTICE**  
> **SwordigoMobile (Swordfare)** is an experimental, independent mobile launcher research project.  
> **Note:** This project may be dropped or paused at any time as most of the OpenSwordigo team is actively engaged on **SwordigoDesktop**.

---

## Overview

**SwordigoMobile** (App Name: **Swordfare**) is a lightweight native Android host layer and custom JNI engine harness designed to execute `libswordigo.so` directly on ARM64 Android devices without emulators or heavy translation layers.

### Key Technical Capabilities
- **Direct ARM64 Native Execution**: Executes `libswordigo.so` natively on AArch64 hardware.
- **`libGlossHook` Inline Hooking**: Integrates `libGlossHook.so` for deep C++ engine instrumentation, function interception, and diagnostics.
- **FBO & Pipeline Enhancements**: Built-in OpenGL ES framebuffer redirection (`glBindFramebuffer`), color/depth clearing, and custom render context hooks.
- **Memory Patches**: Real-time memory patches for Store controllers, ad-bypass routines, and `bad_weak_ptr` exception suppression.
- **Mobile UI Harness**: Custom Java/JNI interface (`Native.java`, `NativeBridge.java`, `MainActivity.java`) exposing TouchFoo callbacks.

---

## Architecture

```
SwordigoMobile (Swordfare)
 ├── app/src/main/cpp/
 │    ├── swd_jni_bridge.cpp      # Native JNI bridge & GlossHook interceptors
 │    ├── swd_jni_bridge.h
 │    ├── swd_launcher_port.cpp   # Engine state manager
 │    └── CMakeLists.txt          # Native NDK CMake configuration
 ├── app/src/main/java/
 │    └── com/swordigo/desktop/
 │         ├── MainActivity.java  # Android Activity & GLSurfaceView harness
 │         ├── NativeBridge.java  # JNI bindings
 │         └── Native.java        # TouchFoo callbacks
 └── app/src/main/assets/         # Vanilla game assets & scene data
```

---

## Building & Installation

### Environment Requirements
- Android NDK `29.0.14206865`
- Gradle `9.3.0`
- Android SDK (`compileSdk 36`, `minSdk 24`)

### Build & Deploy Commands
```bash
export ANDROID_HOME="/path/to/android-sdk"
export ANDROID_NDK_HOME="/path/to/android-ndk"

./build/gradle-9.3.0-bin/gradle-9.3.0/bin/gradle assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
adb shell am start -n com.swordigo.desktop/.MainActivity
```

---

## Credits & License

- **Author / Lead**: MrSinup
- **Engine Baseline**: OpenSwordigo & SwordigoDesktop
- **GlossHook Library**: `libGlossHook.so`
- **Target Game**: Swordigo by TouchFoo

---
*Experimental Research Software — OpenSwordigo Ecosystem*
