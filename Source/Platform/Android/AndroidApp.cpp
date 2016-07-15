#include <Kaleido3D.h>
#include <Core/ModuleCore.h>
#include "AndroidState.h"
#include <Core/App.h>

namespace k3d
{
    namespace Core
    {
        kString GetExecutablePath()
        {
            return "";
        }
    }

    App *RetrieveApp(ANativeActivity *activity)
    {
        return (App *) activity->instance;
    }

    void App::OnStart(ANativeActivity *activity)
    {
        App *app = RetrieveApp(activity);
        app->WriteCmd(APP_CMD_START);
    }

    void App::OnDestroyed(ANativeActivity *nativeActivity)
    {
        App *app = RetrieveApp(nativeActivity);
        app->WriteCmd(APP_CMD_DESTROY);
    }

    void App::OnResume(ANativeActivity *activity) {
        App *app = RetrieveApp(activity);
        app->WriteCmd(APP_CMD_RESUME);
    }

    void App::OnPause(ANativeActivity *activity) {
        App *app = RetrieveApp(activity);
        app->WriteCmd(APP_CMD_PAUSE);
    }

    void App::OnStop(ANativeActivity *activity) {
        App *app = RetrieveApp(activity);
        app->WriteCmd(APP_CMD_STOP);
    }

    void *App::OnSaveInstanceState(ANativeActivity *activity, size_t *outLen) {
        App *app = RetrieveApp(activity);
        //app->WriteCmd(APP_CMD_SAVE_STATE);
    }

    void App::OnWindowFocusChanged(ANativeActivity *activity, int focused) {
        App *app = RetrieveApp(activity);
        app->WriteCmd(APP_CMD_GAINED_FOCUS);
    }

    void App::OnNativeWindowCreated(ANativeActivity *activity, ANativeWindow *window) {
        App *app = RetrieveApp(activity);
//        app->WriteCmd(APP_CMD_INIT_WINDOW);
        ::Concurrency::Mutex::AutoLock autoLock(&app->m_Mutex);
        app->m_PendingWindow = window;
        KLOG(Info, kaleido3d::App, "Window inited.. thread=%s", ::Concurrency::Thread::GetCurrentThreadName().c_str());
        app->OnWindowCreated(window);
        app->m_CondiVar.NotifyAll();
    }

    void App::OnNativeWindowDestroyed(ANativeActivity *activity, ANativeWindow *window) {
        App *app = RetrieveApp(activity);
        app->WriteCmd(APP_CMD_TERM_WINDOW);
    }

    void App::OnInputQueueCreated(ANativeActivity *activity, AInputQueue *queue)
    {
        App * app = RetrieveApp(activity);
        app->SetInputQueue(queue);
    }

    void App::OnInputQueueDestroyed(ANativeActivity *activity, AInputQueue *queue)
    {

    }

    void App::SetInputQueue(AInputQueue *queue) {
        ::Concurrency::Mutex::AutoLock autoLock(&m_Mutex);
        m_PendingInputQueue = queue;
        m_InputQueue = m_PendingInputQueue;
        KLOG(Info, kaleio3d::App,"Attaching input queue to looper");
        AInputQueue_attachLooper(m_InputQueue,
                                 m_Looper, LOOPER_ID_INPUT, NULL,
                                 this); // associate with ALooperPoll
//        WriteCmd(APP_CMD_INPUT_CHANGED);
//        while(m_PendingInputQueue!=m_InputQueue) {
//            m_CondiVar.Wait(&m_Mutex);
//        }
    }

    void App::WriteCmd(int8_t cmd)
    {
        if (write(m_MsgWrite, &cmd, sizeof(cmd)) != sizeof(cmd))
        {
            KLOG(Error, kaleido3d::App, "Failure writing android_app cmd: %s\n", strerror(errno));
        }
    }

    int8_t App::ReadCmd()
    {
        int8_t cmd;
        if (read(m_MsgRead, &cmd, sizeof(cmd)) == sizeof(cmd))
        {
            switch (cmd)
            {
                case APP_CMD_SAVE_STATE:
                    break;
            }
            return cmd;
        }
        else
        {
            KLOG(Info, kaleido3d::App, "No data on command pipe!");
        }
        return -1;
    }

    void App::ProcessCmd()
    {
        int8_t cmd = ReadCmd();
        PreExecCmd(cmd);
        if (onAppCmd != NULL) onAppCmd(this, cmd);
        PostExecCmd(cmd);
    }

    void App::PreExecCmd(int8_t cmd)
    {
        switch (cmd)
        {
            case APP_CMD_INPUT_CHANGED:
                KLOG(Info, kaleio3d::App,"APP_CMD_INPUT_CHANGED\n");
                {
                    ::Concurrency::Mutex::AutoLock autoLock(&m_Mutex);
//                    if (m_InputQueue != NULL)
//                    {
//                        AInputQueue_detachLooper(m_InputQueue);
//                    }
                    m_InputQueue = m_PendingInputQueue;
                    if (m_InputQueue != NULL)
                    {
                        KLOG(Info, kaleio3d::App,"Attaching input queue to looper");
                        AInputQueue_attachLooper(m_InputQueue,
                                                 m_Looper, LOOPER_ID_INPUT, NULL,
                                                 this); // associate with ALooperPoll
                    }
                    m_CondiVar.NotifyAll();
                }
                break;

            case APP_CMD_INIT_WINDOW:
                KLOG(Info, kaleio3d::App,"APP_CMD_INIT_WINDOW");
                {
                    ::Concurrency::Mutex::AutoLock autoLock(&m_Mutex);
                    m_Window = MakeAndroidWindow(m_PendingWindow);
                    m_CondiVar.NotifyAll();
                }
                break;

            case APP_CMD_TERM_WINDOW:
                KLOG(Info, kaleio3d::App,"APP_CMD_TERM_WINDOW\n");
                m_CondiVar.NotifyAll();
                break;

            case APP_CMD_RESUME:
            case APP_CMD_START:
            case APP_CMD_PAUSE:
            case APP_CMD_STOP:
                KLOG(Info, kaleio3d::App,"activityState=%d\n", cmd);
                {
                    ::Concurrency::Mutex::AutoLock autoLock(&m_Mutex);
                }
                break;

            case APP_CMD_CONFIG_CHANGED:
                KLOG(Info, kaleio3d::App,"APP_CMD_CONFIG_CHANGED\n");
                break;

            case APP_CMD_DESTROY:
                KLOG(Info, kaleio3d::App,"APP_CMD_DESTROY\n");
                break;
        }
    }

    void App::PostExecCmd(int8_t cmd)
    {
        switch (cmd) 
        {
            case APP_CMD_TERM_WINDOW:
                KLOG(Info, kaleio3d::App,"APP_CMD_TERM_WINDOW\n");
                {
                    ::Concurrency::Mutex::AutoLock autoLock(&m_Mutex);
                    m_Window = nullptr;
                    m_CondiVar.NotifyAll();
                }
                break;

            case APP_CMD_SAVE_STATE:
                KLOG(Info, kaleio3d::App,"APP_CMD_SAVE_STATE\n");
                {
                    ::Concurrency::Mutex::AutoLock autoLock(&m_Mutex);
                    stateSaved = 1;
                    m_CondiVar.NotifyAll();
                }
                break;
            case APP_CMD_RESUME:
                break;
        }
    }

    /**
     * handle android events here!!!
     * or put it into a message queue
     */
    void App::ProcessInput(Message &msg)
    {
        AInputEvent* event = NULL;
        while (AInputQueue_getEvent(m_InputQueue, &event) >= 0)
        {
            KLOG(Info, kaleido3d::App, "New input event: type=%d\n", AInputEvent_getType(event));
            if (AInputQueue_preDispatchEvent(m_InputQueue, event))
            {
                continue;
            }
            int32_t handled = 0;
            if (onInputEvent != NULL) handled = onInputEvent(this, event);
            AInputQueue_finishEvent(m_InputQueue, event, handled);
        }
    }
}


namespace __android_internal
{
    k3d::App* g_App = nullptr;
    static void appDestroy(k3d::App *android_app);

    static void* appEntry(void *param) {
        k3d::App* app = (k3d::App*)param;
        app->m_Config = AConfiguration_new();
        AConfiguration_fromAssetManager(app->m_Config, app->m_Activity->assetManager);

        ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        ALooper_addFd(looper, app->m_MsgRead, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, app);
        app->m_Looper = looper;

        app->m_Mutex.Lock();
        app->m_IsRunning = 1;
        app->m_CondiVar.NotifyAll();
        app->m_Mutex.UnLock();

        while(!app->OnInit()) {
            app->m_CondiVar.Wait(&app->m_Mutex);
        }
        app->Run();

        appDestroy(app);
        return NULL;
    }

    k3d::App*
    createApplication(
            ANativeActivity *activity,
            k3d::App *androidApp,
            void *savedState, size_t savedStateSize)
    {
        g_App = androidApp;
        androidApp->m_Activity = activity;
        if (savedState != NULL) {
            androidApp->m_SavedState = malloc(savedStateSize);
            androidApp->m_SavedStateSize = savedStateSize;
            memcpy(androidApp->m_SavedState, savedState, savedStateSize);
        }
        // create pipe for communication
        int msgpipe[2];
        if (pipe(msgpipe)) {
            KLOG(Error, kaleido3d::App, "could not create pipe: %s", strerror(errno));
            return NULL;
        }
        androidApp->m_MsgRead = msgpipe[0];
        androidApp->m_MsgWrite = msgpipe[1];

        androidApp->m_Thread = new Concurrency::Thread([androidApp]()
        {
            appEntry(androidApp);
        }, "k3d::appEntry");
        androidApp->m_Thread->Start();

        {
            ::Concurrency::Mutex::AutoLock autoLock(&androidApp->m_Mutex);
            while (!androidApp->m_IsRunning) {
                androidApp->m_CondiVar.Wait(&androidApp->m_Mutex);
            }
        }

        return androidApp;
    }


    static void appDestroy(k3d::App *android_app) {
        ::Concurrency::Mutex::AutoLock autoLock(&android_app->m_Mutex);
        if (android_app->m_InputQueue != NULL) {
            AInputQueue_detachLooper(android_app->m_InputQueue);
        }
        AConfiguration_delete(android_app->m_Config);
        android_app->destroyed = 1;
        android_app->m_CondiVar.NotifyAll();
    }

    const char *getLibraryName(JNIEnv* env, jobject& objectActivityInfo)
    {
        static char name[256];
        jclass classActivityInfo = env->FindClass("android/content/pm/ActivityInfo");
        jfieldID fieldMetaData = env->GetFieldID(classActivityInfo, "metaData", "Landroid/os/Bundle;");
        jobject objectMetaData = env->GetObjectField(objectActivityInfo, fieldMetaData);
        jobject objectName = env->NewStringUTF("kaleido3d.app.lib_name");
        jclass classBundle = env->FindClass("android/os/Bundle");
        jmethodID methodGetString = env->GetMethodID(classBundle, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
        jstring valueString = (jstring) env->CallObjectMethod(objectMetaData, methodGetString, objectName);
        if (valueString == NULL)
        {
            KLOG(Error, kaleido3d::App, "No meta-data 'kaleido3d.app.lib_name' found in AndroidManifest.xml file");
            exit(1);
        }
        const jsize applicationNameLength = env->GetStringUTFLength(valueString);
        const char* applicationName = env->GetStringUTFChars(valueString, NULL);
        if (applicationNameLength >= 256)
        {
            KLOG(Error, kaleido3d::App,"The value of 'kaleido3d.app.lib_name' must not be longer than 255 characters.");
            exit(1);
        }
        strncpy(name, applicationName, applicationNameLength);
        name[applicationNameLength] = '\0';
        env->ReleaseStringUTFChars(valueString, applicationName);
        return name;
    }

    void* loadLibrary(const char* libraryName, JNIEnv* env, jobject& ObjectActivityInfo)
    {
        jclass ClassActivityInfo = env->FindClass("android/content/pm/ActivityInfo");
        jfieldID FieldApplicationInfo = env->GetFieldID(ClassActivityInfo, "applicationInfo", "Landroid/content/pm/ApplicationInfo;");
        jobject ObjectApplicationInfo = env->GetObjectField(ObjectActivityInfo, FieldApplicationInfo);
        jclass ClassApplicationInfo = env->FindClass("android/content/pm/ApplicationInfo");
        jfieldID FieldNativeLibraryDir = env->GetFieldID(ClassApplicationInfo, "nativeLibraryDir", "Ljava/lang/String;");
        jobject ObjectDirPath = env->GetObjectField(ObjectApplicationInfo, FieldNativeLibraryDir);
        jclass ClassSystem = env->FindClass("java/lang/System");
        jmethodID StaticMethodMapLibraryName = env->GetStaticMethodID(ClassSystem, "mapLibraryName", "(Ljava/lang/String;)Ljava/lang/String;");
        jstring LibNameObject = env->NewStringUTF(libraryName);
        jobject ObjectName = env->CallStaticObjectMethod(ClassSystem, StaticMethodMapLibraryName, LibNameObject);
        jclass ClassFile = env->FindClass("java/io/File");
        jmethodID FileConstructor = env->GetMethodID(ClassFile, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
        jobject ObjectFile = env->NewObject(ClassFile, FileConstructor, ObjectDirPath, ObjectName);
        jmethodID MethodGetPath = env->GetMethodID(ClassFile, "getPath", "()Ljava/lang/String;");
        jstring javaLibraryPath = static_cast<jstring>(env->CallObjectMethod(ObjectFile, MethodGetPath));
        const char* libraryPath = env->GetStringUTFChars(javaLibraryPath, NULL);
        void * handle = dlopen(libraryPath, RTLD_NOW | RTLD_GLOBAL);
        if (!handle)
        {
            KLOG(Error, kaleido3d::App,"dlopen(\"%s\"): %s", libraryPath, dlerror());
            exit(1);
        }
        env->ReleaseStringUTFChars(javaLibraryPath, libraryPath);
        return handle;
    }
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
	JavaVM* vm = activity->vm;
	JNIEnv* env = activity->env;
	jobject ObjectNativeActivity = activity->clazz;
	jclass ClassNativeActivity = env->GetObjectClass(ObjectNativeActivity);
	jmethodID MethodGetPackageManager = env->GetMethodID(ClassNativeActivity, "getPackageManager", "()Landroid/content/pm/PackageManager;");
	jobject ObjectPackageManager = env->CallObjectMethod(ObjectNativeActivity, MethodGetPackageManager);
	jmethodID MethodGetIndent = env->GetMethodID(ClassNativeActivity, "getIntent", "()Landroid/content/Intent;");
	jobject ObjectIntent = env->CallObjectMethod(ObjectNativeActivity, MethodGetIndent);
	jclass ClassIntent = env->FindClass("android/content/Intent");
	jmethodID MethodGetComponent = env->GetMethodID(ClassIntent, "getComponent", "()Landroid/content/ComponentName;");
	jobject ObjectComponentName = env->CallObjectMethod(ObjectIntent, MethodGetComponent);
	jclass ClassPackageManager = env->FindClass("android/content/pm/PackageManager");
	jfieldID FieldGET_META_DATA = env->GetStaticFieldID(ClassPackageManager, "GET_META_DATA", "I");
	jint GET_META_DATA = env->GetStaticIntField(ClassPackageManager, FieldGET_META_DATA);
	jmethodID MethodGetActivityInfo = env->GetMethodID(ClassPackageManager, "getActivityInfo", "(Landroid/content/ComponentName;I)Landroid/content/pm/ActivityInfo;");
	jobject ObjectActivityInfo = env->CallObjectMethod(ObjectPackageManager, MethodGetActivityInfo, ObjectComponentName, GET_META_DATA);
    __android_internal::loadLibrary("shadercompiler", env, ObjectActivityInfo);
    __android_internal::loadLibrary("rhi_vulkan", env, ObjectActivityInfo);
    __android_internal::loadLibrary("renderer", env, ObjectActivityInfo);
	void* handle = __android_internal::loadLibrary(__android_internal::getLibraryName(env, ObjectActivityInfo), env, ObjectActivityInfo);
	PFN_NativeActivity_OnCreate createFunc = (PFN_NativeActivity_OnCreate)dlsym(handle, "k3d_Core_App_onCreate");
	if (!createFunc)
	{
        KLOG(Error, kaleido3d::App, "Undefined symbol k3d_Core_App_onCreate");
		exit(1);
	}
	createFunc(activity, savedState, savedStateSize);
}
