package com.touchfoo.swordigo;

import android.content.res.AssetManager;
import android.util.Log;

public class Native {
    private static final String TAG = "TouchFooNative";

    static {
        System.loadLibrary("swd");
        try {
            System.loadLibrary("swordigo");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "libswordigo load managed by libswd loader", e);
        }
    }

    // --- Native functions implemented in C++ (libswordigo.so / libswd.so) ---
    public static native void setupNativeInterface();
    public static native void setFilesDir(String path);
    public static native void setCacheDir(String path);
    public static native void setAssetManager(AssetManager manager);
    public static native void setupApplication(int width, int height, float dpi);
    public static native void handleApplicationLaunch();
    public static native void setApplicationViewSize(int width, int height);
    public static native void updateApplication(float deltaSeconds);
    public static native void drawApplication();
    public static native void handleTouchEvent(int action, float x, float y, int pointerId);
    public static native void applicationDidBecomeActive();
    public static native void applicationDidBecomeInactive();
    public static native void applicationDidEnterBackground();
    public static native void applicationDidEnterForeground();
    public static native void handleBackButtonPress();
    public static native void handleMenuButtonPress();
    public static native String uniqueIdentifier();

    // --- Java static callbacks invoked by libswordigo.so via JNI ---

    public static void startTextInput(String initialText) {
        Log.i(TAG, "startTextInput: " + initialText);
    }

    public static void stopTextInput() {
        Log.i(TAG, "stopTextInput");
    }

    public static void openURL(String url) {
        Log.i(TAG, "openURL: " + url);
    }

    public static void quitApplication() {
        Log.i(TAG, "quitApplication");
    }

    public static String getStoreName() {
        Log.i(TAG, "getStoreName");
        return "GooglePlay";
    }

    public static void queueStoreProductFetch(String productId) {
        Log.i(TAG, "queueStoreProductFetch: " + productId);
    }

    public static void processStoreQueue() {
        Log.i(TAG, "processStoreQueue");
    }

    public static void purchaseStoreProduct(String productId) {
        Log.i(TAG, "purchaseStoreProduct: " + productId);
    }

    public static void consumePurchasesForTesting() {
        Log.i(TAG, "consumePurchasesForTesting");
    }

    public static boolean checkPromotion(String promo) {
        Log.i(TAG, "checkPromotion: " + promo);
        return false;
    }

    public static boolean isAgeKnown() {
        return true;
    }

    public static boolean isAgeOfConsent() {
        return true;
    }

    public static void enteredAge(int age) {
        Log.i(TAG, "enteredAge: " + age);
    }

    public static boolean hasPrivacyConsent() {
        return true;
    }

    public static boolean isExplicitPrivacyConsent() {
        return true;
    }

    public static void receivedPrivacyConsent(boolean consent) {
        Log.i(TAG, "receivedPrivacyConsent: " + consent);
    }

    public static void withdrawPrivacyConsent() {
        Log.i(TAG, "withdrawPrivacyConsent");
    }

    public static void startAdsAndAnalytics() {
        Log.i(TAG, "startAdsAndAnalytics");
    }

    public static String getAnalyticsId() {
        return "SWD_MOBILE";
    }

    public static void showAnalyticsIdPopup() {
        Log.i(TAG, "showAnalyticsIdPopup");
    }

    public static void copyToClipboard(String text, String toast) {
        Log.i(TAG, "copyToClipboard: " + text);
    }

    public static int getPlatformConsentState() {
        return 1;
    }

    public static void showPlatformConsentOptions() {
        Log.i(TAG, "showPlatformConsentOptions");
    }

    public static void prepareReviewFlow() {
        Log.i(TAG, "prepareReviewFlow");
    }

    public static void startReviewFlow() {
        Log.i(TAG, "startReviewFlow");
    }

    public static boolean isGoogleGameServicesAvailable() {
        Log.i(TAG, "isGoogleGameServicesAvailable -> false");
        return false;
    }

    public static void initiateGoogleSignIn() {
        Log.i(TAG, "initiateGoogleSignIn");
    }

    public static void showAchievements() {
        Log.i(TAG, "showAchievements");
    }

    public static void reportAchievement(String achievementId) {
        Log.i(TAG, "reportAchievement: " + achievementId);
    }

    public static void reportAchievementProgress(String achievementId, int progress, boolean completed) {
        Log.i(TAG, "reportAchievementProgress: " + achievementId + ", " + progress + ", " + completed);
    }

    public static void showLeaderboards() {
        Log.i(TAG, "showLeaderboards");
    }

    public static void reportScore(String leaderboardId, long score) {
        Log.i(TAG, "reportScore: " + leaderboardId + ", " + score);
    }

    public static void loadSnapshot(String name, double timestamp) {
        Log.i(TAG, "loadSnapshot: " + name);
    }

    public static void saveSnapshot(String name, byte[] data, String metadata, long time1, long time2) {
        Log.i(TAG, "saveSnapshot: " + name);
    }

    public static void deleteSnapshot() {
        Log.i(TAG, "deleteSnapshot");
    }

    public static void loadInterstitialAd() {
        Log.i(TAG, "loadInterstitialAd");
    }

    public static int getInterstitialAdInterval(String placement, int defaultInterval) {
        return defaultInterval;
    }

    public static boolean showInterstitialAd(double delay) {
        Log.i(TAG, "showInterstitialAd: " + delay);
        return false;
    }
}
