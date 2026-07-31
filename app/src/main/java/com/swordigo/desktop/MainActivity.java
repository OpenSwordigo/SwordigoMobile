package com.swordigo.desktop;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends Activity implements GLSurfaceView.Renderer {

    private static final String TAG = "SwordigoMainActivity";
    private GLSurfaceView mGLView;
    private LinearLayout mLauncherOverlay;
    private TextView mTxtStatus;
    private boolean mEngineInitialized = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Fullscreen & Keep Screen On
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        mGLView = findViewById(R.id.gl_surface_view);
        mLauncherOverlay = findViewById(R.id.launcher_overlay);
        mTxtStatus = findViewById(R.id.txt_status);

        // Force opaque window format to prevent SurfaceFlinger transparent black window bug
        mGLView.getHolder().setFormat(PixelFormat.OPAQUE);

        // Configure GLSurfaceView (GLES 2.0 context, 0 alpha for opaque EGL surface)
        mGLView.setEGLContextClientVersion(2);
        mGLView.setEGLConfigChooser(8, 8, 8, 0, 16, 8);
        mGLView.setRenderer(this);
        mGLView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        // Touch listener for passing input to native engine
        mGLView.setOnTouchListener((v, event) -> {
            int action = event.getActionMasked();
            int pointerIndex = event.getActionIndex();
            int pointerId = event.getPointerId(pointerIndex);
            float x = event.getX(pointerIndex);
            float y = event.getY(pointerIndex);

            NativeBridge.onTouchEvent(action, x, y, pointerId);
            return true;
        });

        Button btnLaunch = findViewById(R.id.btn_launch);
        btnLaunch.setOnClickListener(v -> {
            mLauncherOverlay.setVisibility(View.GONE);
            Toast.makeText(MainActivity.this, "Launching Swordigo Native Engine...", Toast.LENGTH_SHORT).show();
        });

        Button btnSettings = findViewById(R.id.btn_settings);
        btnSettings.setOnClickListener(v -> {
            Toast.makeText(MainActivity.this, "Mobile Launcher Options Open", Toast.LENGTH_SHORT).show();
        });

        // Automatically hide launcher overlay after 1 second so game surface is exposed immediately
        new Handler().postDelayed(() -> {
            if (mLauncherOverlay != null) {
                mLauncherOverlay.setVisibility(View.GONE);
            }
        }, 1000);

        String filesDir = getFilesDir().getAbsolutePath();
        String extDir = getExternalFilesDir(null) != null ?
                getExternalFilesDir(null).getAbsolutePath() : filesDir;

        Log.i(TAG, "Initializing Native Engine with Writable FilesDir: " + filesDir);

        // Extract assets to filesDir in background thread if needed
        new Thread(() -> copyAssetsToFilesDir("")).start();

        mEngineInitialized = NativeBridge.initNativeEngine(filesDir, extDir, getAssets());
        if (mEngineInitialized) {
            mTxtStatus.setText("Engine Loaded. Status: " + NativeBridge.getEngineStatus());
        } else {
            mTxtStatus.setText("Engine load deferred.");
        }
    }

    private void copyAssetsToFilesDir(String path) {
        try {
            String[] list = getAssets().list(path);
            if (list != null && list.length > 0) {
                File targetDir = new File(getFilesDir(), path);
                if (!targetDir.exists()) targetDir.mkdirs();
                for (String file : list) {
                    String subPath = path.isEmpty() ? file : path + "/" + file;
                    copyAssetsToFilesDir(subPath);
                }
            } else if (!path.isEmpty()) {
                File targetFile = new File(getFilesDir(), path);
                if (!targetFile.exists() || targetFile.length() == 0) {
                    InputStream in = getAssets().open(path);
                    FileOutputStream out = new FileOutputStream(targetFile);
                    byte[] buffer = new byte[8192];
                    int read;
                    while ((read = in.read(buffer)) != -1) {
                        out.write(buffer, 0, read);
                    }
                    in.close();
                    out.close();
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "Failed to copy asset path: " + path, e);
        }
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        NativeBridge.onSurfaceCreated();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        NativeBridge.onSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        NativeBridge.onDrawFrame();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
        NativeBridge.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
        NativeBridge.onResume();
    }
}
