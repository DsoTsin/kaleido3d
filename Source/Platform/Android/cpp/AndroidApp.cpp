#include <Kaleido3D.h>
#include <Core/App.h>
#include <android/asset_manager_jni.h>

#define CLZ "com/tsinstudio/kaleido3d/GlobalUtil"

static void runtimeInit(JNIEnv *env, jobject instance, jobject context, jstring libpath, jstring appName)
{
    jclass      ctxcls = env->GetObjectClass(context);
    jmethodID   midast = env->GetMethodID(ctxcls, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject     assets = env->CallObjectMethod(context, midast);
    k3d::GetEnv()->__init__(env, context, appName, libpath, AAssetManager_fromJava(env, assets));
}

static JNINativeMethod nMethods[] = {
        {"initialize",   "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",  (void *)runtimeInit}
};

#define CNTOF(arrays) sizeof(arrays)/sizeof(arrays[0])

jint JNI_OnLoad(JavaVM*vm, void*reserve) {
    JNIEnv* env = NULL;
    jint result = vm->GetEnv((void**) &env, JNI_VERSION_1_4);
    if (result != JNI_OK) {
        return -1;
    }
    if(JNI_OK!=env->RegisterNatives(env->FindClass(CLZ), nMethods, CNTOF(nMethods)))
        return -1;
    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM*vm, void*reserve) {

}