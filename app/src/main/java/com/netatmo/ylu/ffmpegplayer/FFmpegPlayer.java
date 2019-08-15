package com.netatmo.ylu.ffmpegplayer;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class FFmpegPlayer implements SurfaceHolder.Callback{
    private OnPrepareListener onPrepareListener;

    static {
        System.loadLibrary("ffmpegplayer");
    }
    private SurfaceHolder surfaceHolder;
    public void setSurfaceView(SurfaceView surfaceView) {
        if (null != this.surfaceHolder) {
            this.surfaceHolder.removeCallback(this);
        }
        this.surfaceHolder = surfaceView.getHolder();
        this.surfaceHolder.addCallback(this);

    }
    public void start(String path) {
        native_start(path,surfaceHolder.getSurface());
    }

    public void sound(String input, String output){
        native_sound(input, output);
    }

    public native void native_start(String path, Surface surface);

    public native void native_sound(String input, String output);

    public native void native_prepare(String path);

    public void prepare(String path){
        native_prepare(path);
    }

    public void onPrepare(){
        if(this.onPrepareListener != null){
            onPrepareListener.onPrepare();
        }
    }

    public void setOnPrepareListener(OnPrepareListener onPrepareListener){
        this.onPrepareListener = onPrepareListener;
    }


    public interface OnPrepareListener{
        void onPrepare();
    }



    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        this.surfaceHolder = surfaceHolder;
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

}
