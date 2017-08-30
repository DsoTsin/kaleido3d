package com.tsinstudio.kaleido3d;

import android.app.Activity;
import android.os.Bundle;

/**
 * Created by dsotsen on 2016/10/21.
 * NativeActivity
 */
public class NativeActivity extends Activity {

    @Override
    public void onCreate(Bundle param) {
        super.onCreate(param);
        String appName = getResources().getString(getApplicationInfo().labelRes);
        GlobalUtil.initialize(this, getApplicationInfo().nativeLibraryDir, appName);
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

}
