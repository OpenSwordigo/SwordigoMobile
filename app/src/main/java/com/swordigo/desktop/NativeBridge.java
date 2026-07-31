package com.swordigo.desktop;

import android.content.res.AssetManager;
import android.util.Log;

public class NativeBridge {
    private static final String TAG = "SwordigoNativeBridge";

    static {
        try {
            System.loadLibrary("swd");
            Log.i(TAG, "libswd.so loaded successfully!");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "Failed to load libswd.so", e);
        }
    }

    // --- Native Callbacks & Launcher API ---
    public static native boolean initNativeEngine(String filesDir, String extFilesDir, AssetManager assetManager);
    public static native void onSurfaceCreated();
    public static native void onSurfaceChanged(int width, int height);
    public static native void onDrawFrame();
    public static native void onTouchEvent(int action, float x, float y, int pointerId);
    public static native void onPause();
    public static native void onResume();

    // Launcher control
    public static native void setModInstancePath(String path);
    public static native void setFpsCap(int fps);
    public static native String getEngineStatus();
}
