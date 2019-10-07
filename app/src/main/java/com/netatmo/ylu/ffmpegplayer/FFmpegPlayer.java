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
    private OnProgressListener onProgressListener;
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

    public void start2(){
        native_start2();
    }

    public int getDuration(){
        return getDurationNative();
    }

    public native int  getDurationNative();

    public void sound(String input, String output){
        native_sound(input, output);
    }

    public native void native_start(String path, Surface surface);

    public native void native_start2();

    public native void native_sound(String input, String output);

    public native void native_prepare(String path);

    public native void setSufaceNative(Surface surface);

    public void prepare(String path){
        native_prepare(path);
    }

    public native void stopNative();

    public native void releaseNative();

    public void onPrepare(){
        if(this.onPrepareListener != null){
            onPrepareListener.onPrepare();
        }
    }

    public void onError(int errorCode){
        if(this.onPrepareListener != null){
            onPrepareListener.onError(errorCode);
        }
    }

    public void onProgress(int progress){
        if(onProgressListener != null){
            onProgressListener.onProgress(progress);
        }
    }

    public void setOnPrepareListener(OnPrepareListener onPrepareListener){
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener){
        this.onProgressListener = onProgressListener;
    }

    public void release() {
        surfaceHolder.removeCallback(this);
        releaseNative();
    }

    public void stop() {
        stopNative();
    }

    public void seek(int playProgress) {
        seekToNative(playProgress);
    }

    public native void seekToNative(int playProgress);

    public interface OnPrepareListener{
        void onPrepare();
        void onError(int errorCode);
    }

    public interface OnProgressListener{
        void onProgress(int progress);
    }



    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        this.surfaceHolder = surfaceHolder;

        setSufaceNative(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }

}
