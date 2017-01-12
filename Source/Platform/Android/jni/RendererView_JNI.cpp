#include "RendererView.h"
#include <android/log.h>

#define LOGJNIE(...) __android_log_print(ANDROID_LOG_ERROR, "RendererView_JNI", __VA_ARGS__)

static JNINativeMethod renderViewMethods[] = {
        {"nOnSurfaceCreated",   "()V",                          (void *)RendererView::jOnSurfaceCreated},
        {"nOnSurfaceChanged",   "(Landroid/view/Surface;III)V", (void *)RendererView::jOnSurfaceChanged},
        {"nOnSurfaceDestroyed", "()V",                          (void *)RendererView::jOnSurfaceDestroyed},
        {"nLoop",               "()V",                          (void *)RendererView::jLoop}
};

#define CNTOF(arrays) sizeof(arrays)/sizeof(arrays[0])

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = vm->GetEnv((void**) &env, JNI_VERSION_1_4);
    if (result != JNI_OK) {
        LOGJNIE("GetEnv failed");
        return result;
    }
    result = env->RegisterNatives(
            env->FindClass(JCLASS_RendererView),
            renderViewMethods,
            CNTOF(renderViewMethods));
    if(JNI_OK != result)
    {
        LOGJNIE("Register Class(%s) failed", JCLASS_RendererView);
        return result;
    }

    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{

}