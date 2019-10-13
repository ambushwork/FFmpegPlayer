package com.netatmo.ylu.ffmpegplayer

import android.app.Activity
import android.content.Intent
import android.hardware.Camera
import android.net.Uri
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.SurfaceView
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File
import android.support.v4.content.FileProvider
import android.view.View
import android.view.View.VISIBLE
import android.widget.SeekBar

class MainActivity : AppCompatActivity() {

    private lateinit var player: FFmpegPlayer
    private var path : String? = null
    private val isTouch : Boolean ? = null
    private lateinit var cameraHelper : CameraHelper

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        player = FFmpegPlayer()

        cameraHelper = CameraHelper(this, Camera.CameraInfo.CAMERA_FACING_BACK, 480, 800)
        cameraHelper.setOnChangedSizeListener { w, h ->
            //do nothing
        }
        cameraHelper.setPreviewCallback { data, camera ->

        }
        cameraHelper.setPreviewDisplay(surfaceView.holder)
        //player.setSurfaceView(surfaceView)
        player.setOnProgressListener {
            val duration = player.duration
            if(duration != 0){
                runOnUiThread{
                    seek_bar.progress = it/duration * 100
                }
            }
        }
        player.setOnPrepareListener(object : FFmpegPlayer.OnPrepareListener {
            override fun onPrepare() {
                runOnUiThread{
                    button_play.visibility = VISIBLE
                }
                val duration = player.duration;
                if(duration != 0){
                    runOnUiThread{
                        seek_bar.visibility = VISIBLE
                    }
                }
            }

            override fun onError(errorCode: Int) {
                TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
            }

        })
        button_select.setOnClickListener {
            chooseFile()

        }
        button_play.setOnClickListener {
            path?.let {
                //playFile(it)
                player.start2()
            }

        }
        button_transform.setOnClickListener {
            path?.let{
                transform(it)
            }
        }
        button_prepare.setOnClickListener {
            path?.let{
                prepare(it)
            }
        }
        seek_bar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener{
            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
                seekBar.progress = progress
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                val progress = seek_bar.progress
                val duration = player .duration
                val playProgress= progress * duration /100
                player.seek(playProgress)

            }

        })
        btn_start_live.setOnClickListener {

        }
        text_info.text = player.stringFromJNI()
    }

    private fun chooseFile() {
        val intent = Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*").addCategory(Intent.CATEGORY_OPENABLE)
        startActivityForResult(Intent.createChooser(intent, "Choose File"), 1)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        var uri : Uri? = null
        if (resultCode == Activity.RESULT_OK) {
            if (requestCode == 1) {
                uri = data?.data
            }
            path = FileSearch.getPath(this, uri)
            Log.d("Uri",path)
        } else {
            Log.e("Error", "onActivityResult() error, resultCode: $resultCode")
        }
        super.onActivityResult(requestCode, resultCode, data)
    }

    private fun  playFile(path: String){
        val video = File(path)
        if(video.exists()){
            player.start(video.absolutePath)
        } else {
            Log.e("Main", "File doesn't exist!")
        }
    }

    private fun transform(path: String){
        val audio = File(path)
        if(audio.exists()){
            val file = File("/storage/emulated/0/output.pcm")
            player.sound(path, file.absolutePath)
        }
    }

    private fun prepare(path: String){
        player.prepare(path)
    }

    override fun onDestroy() {
        super.onDestroy()
        player.release()
    }

    override fun onStop() {
        super.onStop()
        player.stop()
    }


}
