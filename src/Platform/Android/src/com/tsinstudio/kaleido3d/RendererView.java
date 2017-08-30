package com.tsinstudio.kaleido3d;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by dsotsen on 2016/10/21.
 * RendererView
 */
public class RendererView extends SurfaceView implements SurfaceHolder.Callback {

    public static final String TAG = "RendererView";

    private long nativePtr;
    private HandlerThread drawThread;
    private Handler drawHandler;
    private Runnable drawRunner;
    public RendererView(Context context) {
        super(context);
        init();
    }

    private boolean isVisible;

    public RendererView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public RendererView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        getHolder().addCallback(this);
        drawThread = new HandlerThread("kRenderThread", android.os.Process.THREAD_PRIORITY_FOREGROUND);
        if (!drawThread.isAlive()) {
            drawThread.start();
        }
        drawHandler = new Handler(drawThread.getLooper());
        drawRunner = new Runnable() {
            @Override
            public void run() {
                RendererView.this.nLoop();
                if(isVisible)
                    doFrame();
            }
        };
    }


    private void doFrame() {
        drawHandler.postDelayed(drawRunner, 16);
        if (!drawThread.isAlive()) {
            drawThread.run();
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        isVisible = false;
        if (drawThread != null) {
            drawThread.quit();
            drawThread = null;
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v(TAG, "surfaceCreated()");
        nOnSurfaceCreated();
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        nOnSurfaceChanged(holder.getSurface(), format, width, height);
        isVisible = true;
        doFrame();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        nOnSurfaceDestroyed();
        isVisible = false;
    }

    private native void nOnSurfaceCreated();
    private native void nOnSurfaceChanged(Surface s, int format, int width, int height);
    private native void nOnSurfaceDestroyed();

    private native void nLoop();

}
