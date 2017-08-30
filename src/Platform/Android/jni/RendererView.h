#ifndef __RendererView__
#define __RendererView__

#include <jni.h>
#include <android/native_window.h>
#include <Core/App.h>

#define JCLASS_RendererView "com/tsinstudio/kaleido3d/RendererView"

class RendererView {
public:
    RendererView(jobject surfaceView);
    ~RendererView();

    void initialize();
    void onChanged(ANativeWindow*, jint format, jint width, jint height);
    void loop();
    void destroy();

    static void jOnSurfaceCreated(JNIEnv* env, jobject instance);
    static void jOnSurfaceChanged(JNIEnv* env, jobject instance, jobject surface,
                                  jint format, jint width, jint height);

    static void jOnSurfaceDestroyed(JNIEnv* env, jobject instance);
    static void jLoop(JNIEnv* env, jobject instance);
private:
    k3d::App * app;
};

#endif