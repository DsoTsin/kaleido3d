package com.tsinstudio.kaleido3d;

import android.content.Context;
import android.content.res.AssetManager;

public class GlobalUtil {
    public static native void initialize(Context context, String libPath, String appName);
    //public static native void destroy();

    static {
        System.loadLibrary("Core");
    }
}
