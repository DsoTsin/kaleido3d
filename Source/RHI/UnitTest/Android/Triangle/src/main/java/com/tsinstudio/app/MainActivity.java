package com.tsinstudio.app;

import android.os.Bundle;
import com.tsinstudio.kaleido3d.NativeActivity;

public class MainActivity extends NativeActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_k);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    static {
        System.loadLibrary("RHI-UnitTest-3.Triangle");
    }
}
