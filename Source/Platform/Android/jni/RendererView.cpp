#include "RendererView.h"
#include <android/native_window_jni.h>

void RendererView::jOnSurfaceChanged(JNIEnv *env, jobject instance, jobject surface, jint format, jint width, jint height) {
    jclass clz = env->FindClass(JCLASS_RendererView);
    jfieldID fld = env->GetFieldID(clz, "nativePtr", "J");
    auto nSurface = (RendererView*)env->GetLongField(instance, fld);
    if(nSurface) {
        nSurface->onChanged(ANativeWindow_fromSurface(env, surface), format, width, height);
    }
}

void RendererView::jOnSurfaceDestroyed(JNIEnv *env, jobject instance) {
    jclass clz = env->FindClass(JCLASS_RendererView);
    jfieldID fld = env->GetFieldID(clz, "nativePtr", "J");
    auto nSurface = (RendererView*)env->GetLongField(instance, fld);
    nSurface->destroy();
}


void RendererView::jOnSurfaceCreated(JNIEnv *env, jobject instance) {
    jclass clz = env->FindClass(JCLASS_RendererView);
    jfieldID fld = env->GetFieldID(clz, "nativePtr", "J");
    auto surface = new RendererView(instance);
    env->SetLongField(instance, fld, (long)surface);
}

RendererView::RendererView(jobject surfaceView) : app(nullptr) {
    initialize();
}

RendererView::~RendererView() {
    destroy();
}

void RendererView::onChanged(ANativeWindow *window, jint format, jint width, jint height) {
    if(!app) {
        app = ACreateApp(window, width, height);
        app->OnInit();
    }
}

void RendererView::initialize() {

}

void RendererView::destroy() {
}

void RendererView::loop() {
    if(app) {
        app->Run();
    }
}

void RendererView::jLoop(JNIEnv *env, jobject instance) {
    jclass clz = env->FindClass(JCLASS_RendererView);
    jfieldID fld = env->GetFieldID(clz, "nativePtr", "J");
    auto nSurface = (RendererView*)env->GetLongField(instance, fld);
    nSurface->loop();
}






