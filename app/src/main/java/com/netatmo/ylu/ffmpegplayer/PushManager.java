package com.netatmo.ylu.ffmpegplayer;

import android.app.Activity;
import android.support.annotation.NonNull;
import android.view.SurfaceHolder;
import org.jetbrains.annotations.Nullable;

public class PushManager {
    private VideoChannel videoChannel;
    public PushManager(Activity activity,int cameraId, int width, int height, int bitrate){
        videoChannel = new VideoChannel(activity, cameraId, width,height, bitrate);
        native_init();
    }

    public void setPreviewDisplay(@Nullable SurfaceHolder holder) {
        videoChannel.setPreviewDisplay(holder);
    }

    public void startLive(@NonNull String s) {
        native_start_live(s);
        videoChannel.startLive();
    }

    public void stopLive(){
        videoChannel.stopLive();
        native_stop();
    }

    public native void native_stop();


    public native void native_init();

    public native void native_start_live(String path);
}
