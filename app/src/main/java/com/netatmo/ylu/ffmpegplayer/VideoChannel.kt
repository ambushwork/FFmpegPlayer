package com.netatmo.ylu.ffmpegplayer

import android.app.Activity
import android.hardware.Camera
import android.view.SurfaceHolder

class VideoChannel (val pushManager: PushManager,activity: Activity, cameraId : Int, width : Int, height : Int, val fps : Int,val bitrate:Int) :


    Camera.PreviewCallback, CameraHelper.OnChangedSizeListener {
    override fun onChanged(w: Int, h: Int) {
        pushManager.native_initVideoEncoder(w, h, fps, bitrate);
    }

    override fun onPreviewFrame(data: ByteArray?, camera: Camera?) {
        if(isLive){
            pushManager.native_pushVideo(data)
        }
    }

    var isLive = false


    fun setPreviewDisplay(holder: SurfaceHolder?) {
        cameraHelper.setPreviewDisplay(holder)
        cameraHelper.setPreviewCallback(this)
        cameraHelper.setOnChangedSizeListener(this)
    }



    fun startLive() {
        isLive = true
    }

    fun stopLive() {
        isLive = false
    }


    private val cameraHelper : CameraHelper = CameraHelper(activity, cameraId, width, height)



}