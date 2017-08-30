#include "CoreMinimal.h"
#include <XPlatform/App.h>
#include <unistd.h>
#include <android/asset_manager_jni.h>

#define CLZ "com/tsinstudio/kaleido3d/GlobalUtil"

static void runtimeInit(JNIEnv *env, jobject instance, jobject context, jstring libpath, jstring appName)
{
    jclass      ctxcls = env->GetObjectClass(context);
    jmethodID   midast = env->GetMethodID(ctxcls, "getAssets", "()Landroid/content/res/AssetManager;");
    jobject     assets = env->CallObjectMethod(context, midast);
    k3d::GetEnv().__init__(env, context, appName, libpath, AAssetManager_fromJava(env, assets));
}

static JNINativeMethod nMethods[] = {
        {"initialize",   "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)V",  (void *)runtimeInit}
};

namespace k3d
{
    using namespace os;

    struct EnvironmentImpl
    {
        String InstanceName;
        String ExecutableDir;
        String DataDir;

        EnvironmentImpl()
                : InstanceName(64)
                , ExecutableDir(400)
                , DataDir(400)
        {
            String ProcessPath(1024);
            readlink("/proc/self/exe", ProcessPath.Data(), 1024);
            ProcessPath.ReCalculate();
            auto Pos = ProcessPath.FindLastOf("/");
            InstanceName = ProcessPath.SubStr(Pos + 1, ProcessPath.Length() - Pos - 1);
            ExecutableDir = ProcessPath.SubStr(0, Pos);
            //DataDir =
            String CurrentPath = ExecutableDir;
            while(!Exists(Join(CurrentPath, "Data").CStr()))
            {
                auto NewPos = CurrentPath.FindLastOf("/");
                if (NewPos == String::npos)
                {
                    DataDir = ".";
                    break;
                }
                CurrentPath = CurrentPath.SubStr(0, NewPos);
                String FindPath = Join(CurrentPath, "Data");
                if (Exists(FindPath.CStr())) {
                    DataDir = FindPath;
                    break;
                }
            }
        }
    };

    Environment::Environment()
    {
        d = new EnvironmentImpl;
    }

    Environment::~Environment()
    {
        delete d;
    }

    String Environment::GetLogDir() const
    {
        return d->ExecutableDir;
    }

    String Environment::GetModuleDir() const
    {
        return d->ExecutableDir;
    }

    String Environment::GetDataDir()const
    {
        return d->DataDir;
    }

    String Environment::GetInstanceName() const
    {
        return d->InstanceName;
    }
}

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