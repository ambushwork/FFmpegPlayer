package com.netatmo.ylu.ffmpegplayer

import android.app.Activity
import android.view.SurfaceHolder

class VideoChannel (activity: Activity, cameraId : Int, width : Int, height : Int,val bitrate:Int){
    fun setPreviewDisplay(holder: SurfaceHolder?) {
        cameraHelper.setPreviewDisplay(holder)
    }

    fun startLive() {


    }

    fun stopLive() {

    }


    private val cameraHelper : CameraHelper = CameraHelper(activity, cameraId, width, height)



}