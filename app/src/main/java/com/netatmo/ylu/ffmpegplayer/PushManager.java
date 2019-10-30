package com.netatmo.ylu.ffmpegplayer;

import android.app.Activity;
import android.support.annotation.NonNull;
import android.view.SurfaceHolder;
import org.jetbrains.annotations.Nullable;

public class PushManager {
    private VideoChannel videoChannel;
    private AudioChannel audioChannel;
    public PushManager(Activity activity,int cameraId, int width, int height, int fps, int bitrate){
        videoChannel = new VideoChannel(this,activity, cameraId, width,height, fps, bitrate);
        audioChannel = new AudioChannel(this);
        native_init();
    }

    public void setPreviewDisplay(@Nullable SurfaceHolder holder) {
        videoChannel.setPreviewDisplay(holder);
    }

    public void startLive(@NonNull String s) {
        native_start_live(s);
        videoChannel.startLive();
        audioChannel.startLive();
    }

    public void stopLive(){
        videoChannel.stopLive();
        audioChannel.stopLive();
        native_stop();
    }

    public native void native_stop();

    public native void native_init();

    public native void native_start_live(String path);

    public native void native_pushVideo(byte[] data);

    public native void native_initAudioEncoder(int sampleRate,int numChannels);

    public native int getInputSamples();

    public  native  void native_pushAudio(byte[] bytes);

    public native void native_initVideoEncoder(int w, int h, int fps, int bitrate);
}
