package com.tsinstudio.kaleido3d;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Process;
import android.view.View;

import java.io.RandomAccessFile;

/**
 * Created by alexqzhou on 2016/10/31.
 */

public class CPUMonitorView extends View {

    public CPUMonitorView(Context context) {
        super(context);
        init();
    }

    private int numCores = 0;
    private float[] usages;
    Paint paint = new Paint();

    private void init() {
        numCores = Runtime.getRuntime().availableProcessors();
        usages = new float[numCores];
    }

    Rect tmp = new Rect();

    @Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        int width = getWidth();
        int height = getHeight();
        int columnWidth = width / numCores;
        canvas.save();
        for(int i=0; i<numCores; i++) {
            if(i>usages.length)
                break;
            float usage = usages[i];
            if(usage>0.0f) {
                paint.setColor(Color.BLUE);
            }
            if(usage>0.5f) {
                paint.setColor(Color.GREEN);
            }
            if(usage>0.9f) {
                paint.setColor(Color.RED);
            }
            tmp.set(0, (int)(height*usage), columnWidth, 0);
            canvas.drawRect(tmp, paint);
            canvas.translate(columnWidth, 0);
        }
        canvas.restore();
    }

    public void update() {
        String profilePath = "/proc/" + Process.myPid() + "/stat";
    }
}
